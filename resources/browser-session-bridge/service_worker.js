// CodexBarX Browser Session Bridge — Service Worker
// Connects to localhost WebSocket and handles cookie imports.

const DEFAULT_PORT = 18765;
const PROTOCOL_VERSION = 1;
const RECONNECT_INITIAL_DELAY_MS = 1000;
const RECONNECT_MAX_DELAY_MS = 30000;
const PING_INTERVAL_MS = 15000;
const PONG_TIMEOUT_MS = 30000;

let socket = null;
let reconnectTimer = null;
let pingTimer = null;
let pongTimer = null;
let reconnectDelay = RECONNECT_INITIAL_DELAY_MS;
let profileInstanceId = null;
let profileAlias = '';
let serverPort = DEFAULT_PORT;
let connectionState = 'disconnected';
let lastImportResult = null;

// --- UUID v4 generator ---
function generateUUID() {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
    const r = Math.random() * 16 | 0;
    const v = c === 'x' ? r : (r & 0x3 | 0x8);
    return v.toString(16);
  });
}

// --- Storage helpers ---
async function loadProfile() {
  const stored = await chrome.storage.local.get(['profileInstanceId', 'profileAlias']);
  profileInstanceId = stored.profileInstanceId || generateUUID();
  profileAlias = stored.profileAlias || '';
  // Persist generated ID
  if (!stored.profileInstanceId) {
    await chrome.storage.local.set({ profileInstanceId });
  }
}

async function saveProfileAlias(alias) {
  profileAlias = alias;
  await chrome.storage.local.set({ profileAlias: alias });
}

// --- Runtime config ---
async function loadRuntimeConfig() {
  try {
    // In a real unpacked extension loaded from filesystem, runtime.json is in the same directory.
    // Service worker cannot directly read filesystem, but we can embed defaults here.
    // The InstallService writes runtime.json next to the extension files; if we need dynamic
    // port discovery we can use chrome.runtime.getURL('runtime.json') but fetch from extension
    // package requires the file to be web_accessible_resource. For now we use default port.
    const url = chrome.runtime.getURL('runtime.json');
    const resp = await fetch(url);
    if (resp.ok) {
      const cfg = await resp.json();
      if (cfg.serverPort) serverPort = cfg.serverPort;
    }
  } catch (e) {
    // Fallback to default port
  }
}

// --- Browser family detection ---
function detectBrowserFamily() {
  const ua = navigator.userAgent.toLowerCase();
  if (ua.includes('edg/')) return 'edge';
  if (ua.includes('opr/') || ua.includes('opera')) return 'opera';
  if (ua.includes('vivaldi')) return 'vivaldi';
  if (ua.includes('brave')) return 'brave';
  return 'chrome';
}

function detectBrowserVersion() {
  const ua = navigator.userAgent;
  const m = ua.match(/(Chrome|Edg|OPR|Vivaldi)\/([0-9.]+)/);
  return m ? m[2] : '0.0.0.0';
}

// --- Connection state ---
function setConnectionState(state) {
  connectionState = state;
  chrome.runtime.sendMessage({ type: 'connectionStateChanged', state }).catch(() => {});
}

// --- WebSocket ---
function connect() {
  if (socket) return;

  setConnectionState('connecting');
  const wsUrl = `ws://127.0.0.1:${serverPort}`;
  socket = new WebSocket(wsUrl);

  socket.onopen = async () => {
    reconnectDelay = RECONNECT_INITIAL_DELAY_MS;
    setConnectionState('connected');
    startPingTimer();
    await sendRegisterClient();
  };

  socket.onmessage = (event) => {
    handleMessage(event.data);
  };

  socket.onclose = () => {
    cleanupSocket();
    setConnectionState('disconnected');
    scheduleReconnect();
  };

  socket.onerror = () => {
    // onclose will fire next
  };
}

function cleanupSocket() {
  if (pingTimer) { clearInterval(pingTimer); pingTimer = null; }
  if (pongTimer) { clearTimeout(pongTimer); pongTimer = null; }
  if (socket) {
    socket.close();
    socket = null;
  }
}

function scheduleReconnect() {
  if (reconnectTimer) return;
  reconnectTimer = setTimeout(() => {
    reconnectTimer = null;
    connect();
  }, reconnectDelay);
  reconnectDelay = Math.min(reconnectDelay * 2, RECONNECT_MAX_DELAY_MS);
}

function startPingTimer() {
  if (pingTimer) clearInterval(pingTimer);
  pingTimer = setInterval(() => {
    if (socket && socket.readyState === WebSocket.OPEN) {
      sendMessage({ type: 'Ping', payload: {} });
      // If no pong within timeout, close and reconnect
      if (pongTimer) clearTimeout(pongTimer);
      pongTimer = setTimeout(() => {
        if (socket) socket.close();
      }, PONG_TIMEOUT_MS);
    }
  }, PING_INTERVAL_MS);
}

// --- Message helpers ---
function sendMessage(msg) {
  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify(msg));
  }
}

async function sendRegisterClient() {
  sendMessage({
    type: 'RegisterClient',
    payload: {
      protocolVersion: PROTOCOL_VERSION,
      extensionId: chrome.runtime.id,
      browserFamily: detectBrowserFamily(),
      browserVersion: detectBrowserVersion(),
      profileInstanceId: profileInstanceId,
      profileAlias: profileAlias,
      supportsCookies: true,
      supportsLocalStorage: false,
      incognito: false
    }
  });
}

// --- Message handlers ---
function handleMessage(text) {
  try {
    const msg = JSON.parse(text);
    switch (msg.type) {
      case 'RegisterAck':
        // Desktop acknowledged registration; payload may contain supported provider specs
        break;
      case 'RequestImport':
        handleRequestImport(msg.payload);
        break;
      case 'Ping':
        sendMessage({ type: 'Pong', payload: {} });
        break;
      case 'Pong':
        if (pongTimer) { clearTimeout(pongTimer); pongTimer = null; }
        break;
      case 'Error':
        console.error('[Bridge] Server error:', msg.payload);
        break;
    }
  } catch (e) {
    console.error('[Bridge] Failed to parse message:', e);
  }
}

async function handleRequestImport(payload) {
  const { requestId, providerId, materialKind, domains, cookieNames } = payload;

  if (materialKind !== 'Cookies') {
    sendImportResult(requestId, providerId, [], 'unsupported_material_kind');
    return;
  }

  const cookies = [];
  try {
    for (const domain of domains) {
      const all = await chrome.cookies.getAll({ domain });
      for (const c of all) {
        // If cookieNames is non-empty, only include named cookies
        if (cookieNames && cookieNames.length > 0 && !cookieNames.includes(c.name)) {
          continue;
        }
        cookies.push({
          name: c.name,
          value: c.value,
          domain: c.domain,
          path: c.path,
          sameSite: c.sameSite,
          storeId: c.storeId,
          secure: c.secure,
          httpOnly: c.httpOnly,
          hostOnly: c.hostOnly,
          session: c.session,
          expirationDateUtc: c.expirationDate ? new Date(c.expirationDate * 1000).toISOString() : null,
          partitionKey: c.partitionKey || ''
        });
      }
    }

    sendImportResult(requestId, providerId, cookies, null);
  } catch (err) {
    sendImportResult(requestId, providerId, [], err.message || 'cookie_fetch_failed');
  }
}

function sendImportResult(requestId, providerId, cookies, error) {
  const payload = {
    requestId: requestId,
    providerId: providerId,
    success: !error,
    cookies: cookies,
    localStorage: {},
    capturedAtUtc: new Date().toISOString(),
    sourceReason: 'manual_request',
    schemaVersion: 1
  };
  if (error) {
    payload.errorCode = typeof error === 'string' ? error : 'unknown_error';
  }

  lastImportResult = { providerId, success: !error, capturedAt: payload.capturedAtUtc };
  chrome.runtime.sendMessage({ type: 'importResult', result: lastImportResult }).catch(() => {});

  sendMessage({ type: 'ImportResult', payload: payload });
}

// --- Popup / runtime messaging ---
chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  if (message.type === 'getState') {
    sendResponse({
      connectionState,
      profileInstanceId,
      profileAlias,
      lastImportResult
    });
    return true;
  }
  if (message.type === 'setAlias') {
    saveProfileAlias(message.alias).then(() => {
      // Reconnect to send updated alias
      cleanupSocket();
      if (reconnectTimer) { clearTimeout(reconnectTimer); reconnectTimer = null; }
      connect();
      sendResponse({ ok: true });
    });
    return true;
  }
  if (message.type === 'reconnect') {
    cleanupSocket();
    if (reconnectTimer) { clearTimeout(reconnectTimer); reconnectTimer = null; }
    reconnectDelay = RECONNECT_INITIAL_DELAY_MS;
    connect();
    sendResponse({ ok: true });
    return true;
  }
});

// --- Startup ---
(async function init() {
  await loadProfile();
  await loadRuntimeConfig();
  connect();
})();
