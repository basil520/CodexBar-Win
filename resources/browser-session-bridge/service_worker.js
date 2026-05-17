// CodexBarX Browser Session Bridge - Service Worker
// Connects to localhost WebSocket and imports browser session material.

importScripts('protocol.js');

const DEFAULT_PORT = 18765;
const RECONNECT_INITIAL_DELAY_MS = 1000;
const RECONNECT_MAX_DELAY_MS = 30000;
const PING_INTERVAL_MS = 15000;
const PONG_TIMEOUT_MS = 30000;
const EXTENSION_BUILD = '2026.05.17-cookie-url-query';

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

function generateUUID() {
  return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, (c) => {
    const r = Math.random() * 16 | 0;
    const v = c === 'x' ? r : (r & 0x3 | 0x8);
    return v.toString(16);
  });
}

async function loadProfile() {
  const stored = await chrome.storage.local.get(['profileInstanceId', 'profileAlias']);
  profileInstanceId = stored.profileInstanceId || generateUUID();
  profileAlias = stored.profileAlias || 'Default';
  if (!stored.profileInstanceId) {
    await chrome.storage.local.set({ profileInstanceId });
  }
  if (!stored.profileAlias) {
    await chrome.storage.local.set({ profileAlias });
  }
}

async function saveProfileAlias(alias) {
  profileAlias = (alias || '').trim() || 'Default';
  await chrome.storage.local.set({ profileAlias });
}

async function loadRuntimeConfig() {
  try {
    const resp = await fetch(chrome.runtime.getURL('runtime.json'));
    if (resp.ok) {
      const cfg = await resp.json();
      if (Number.isInteger(cfg.serverPort)) serverPort = cfg.serverPort;
    }
  } catch (e) {
    serverPort = DEFAULT_PORT;
  }
}

async function detectBrowserFamily() {
  if (navigator.brave && await navigator.brave.isBrave()) return 'brave';
  const ua = navigator.userAgent.toLowerCase();
  if (ua.includes('edg/')) return 'edge';
  if (ua.includes('opr/') || ua.includes('opera')) return 'opera';
  if (ua.includes('vivaldi')) return 'vivaldi';
  return 'chrome';
}

function detectBrowserVersion() {
  const ua = navigator.userAgent;
  const m = ua.match(/(Chrome|Edg|OPR|Vivaldi)\/([0-9.]+)/);
  return m ? m[2] : '0.0.0.0';
}

function setConnectionState(state) {
  connectionState = state;
  chrome.runtime.sendMessage({ type: 'connectionStateChanged', state }).catch(() => {});
}

function connect() {
  if (socket && (socket.readyState === WebSocket.CONNECTING || socket.readyState === WebSocket.OPEN)) {
    return;
  }

  setConnectionState('connecting');
  socket = new WebSocket(`ws://127.0.0.1:${serverPort}`);

  socket.onopen = async () => {
    reconnectDelay = RECONNECT_INITIAL_DELAY_MS;
    setConnectionState('connected');
    startPingTimer();
    await sendRegisterClient();
  };

  socket.onmessage = (event) => handleMessage(event.data);

  socket.onclose = () => {
    cleanupSocket();
    setConnectionState('disconnected');
    scheduleReconnect();
  };

  socket.onerror = () => {
    if (socket) socket.close();
  };
}

function cleanupSocket() {
  if (pingTimer) { clearInterval(pingTimer); pingTimer = null; }
  if (pongTimer) { clearTimeout(pongTimer); pongTimer = null; }
  if (socket) {
    const oldSocket = socket;
    socket = null;
    oldSocket.close();
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
      sendMessage({ type: BridgeProtocol.message.ping });
      if (pongTimer) clearTimeout(pongTimer);
      pongTimer = setTimeout(() => {
        if (socket) socket.close();
      }, PONG_TIMEOUT_MS);
    }
  }, PING_INTERVAL_MS);
}

function sendMessage(msg) {
  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify(msg));
  }
}

async function sendRegisterClient() {
  sendMessage({
    type: BridgeProtocol.message.registerClient,
    protocolVersion: BridgeProtocol.version,
    extensionBuild: EXTENSION_BUILD,
    extensionId: chrome.runtime.id,
    browserFamily: await detectBrowserFamily(),
    browserVersion: detectBrowserVersion(),
    profileInstanceId,
    profileAlias,
    incognito: false,
    capabilities: {
      cookies: true,
      localStorage: true,
      codexUsageSnapshot: true
    }
  });
}

function handleMessage(text) {
  try {
    const msg = JSON.parse(text);
    switch (msg.type) {
      case BridgeProtocol.message.registerAck:
        break;
      case BridgeProtocol.message.requestImport:
        handleRequestImport(msg);
        break;
      case BridgeProtocol.message.ping:
        sendMessage({ type: BridgeProtocol.message.pong });
        break;
      case BridgeProtocol.message.pong:
        if (pongTimer) { clearTimeout(pongTimer); pongTimer = null; }
        break;
      case BridgeProtocol.message.error:
        console.error('[Bridge] Server error:', msg);
        break;
      default:
        console.warn('[Bridge] Unknown message:', msg.type);
        break;
    }
  } catch (e) {
    console.error('[Bridge] Failed to parse message:', e);
  }
}

async function handleRequestImport(payload) {
  const { requestId, providerId, materialKind } = payload;

  if (providerId === 'codex') {
    await handleCodexImport(requestId, payload);
    return;
  }

  if (materialKind === BridgeProtocol.material.cookies) {
    await handleCookieImport(requestId, providerId, payload.domains || [], payload.cookieNames || []);
  } else if (materialKind === BridgeProtocol.material.localStorage) {
    await handleLocalStorageImport(requestId, providerId, payload.origin, payload.localStorageKeys || []);
  } else if (materialKind === BridgeProtocol.material.hybrid) {
    await handleCookieImport(requestId, providerId, payload.domains || [], payload.cookieNames || []);
    await handleLocalStorageImport(requestId, providerId, payload.origin, payload.localStorageKeys || []);
  } else {
    sendImportResult(requestId, providerId, [], 'unsupported_material_kind', {});
  }
}

async function collectCookies(domains, cookieNames) {
  const cookies = [];
  const seen = new Set();
  for (const domain of domains) {
    const all = await queryCookiesForDomain(domain);
    for (const c of all) {
      if (cookieNames.length > 0 && !cookieNames.includes(c.name)) continue;
      const dedupeKey = [
        c.storeId || '',
        c.domain || '',
        c.path || '',
        c.name || '',
        c.partitionKey ? JSON.stringify(c.partitionKey) : ''
      ].join('|');
      if (seen.has(dedupeKey)) continue;
      seen.add(dedupeKey);
      const cookieRecord = {
        name: c.name,
        value: c.value,
        domain: c.domain,
        path: c.path,
        sameSite: c.sameSite,
        storeId: c.storeId,
        secure: c.secure,
        httpOnly: c.httpOnly,
        hostOnly: c.hostOnly,
        session: c.session
      };
      if (typeof c.expirationDate === 'number') {
        cookieRecord.expirationDate = c.expirationDate;
      }
      if (c.partitionKey) {
        cookieRecord.partitionKey = c.partitionKey;
      }
      cookies.push(cookieRecord);
    }
  }
  return cookies;
}

function normalizeCookieDomain(domain) {
  return String(domain || '')
    .trim()
    .replace(/^https?:\/\//i, '')
    .replace(/\/.*$/, '')
    .replace(/^\*\./, '')
    .replace(/^\./, '')
    .toLowerCase();
}

function cookieQueryUrlsForDomain(domain) {
  const host = normalizeCookieDomain(domain);
  return host ? [`https://${host}/`] : [];
}

async function queryCookiesForDomain(domain) {
  const host = normalizeCookieDomain(domain);
  if (!host) return [];

  const output = [];
  for (const query of [{ domain: host }, { domain: `.${host}` }]) {
    try {
      const found = await chrome.cookies.getAll(query);
      output.push(...found);
    } catch (err) {
      console.warn('[Bridge] Cookie query failed:', query, err);
    }
  }
  for (const url of cookieQueryUrlsForDomain(host)) {
    try {
      const found = await chrome.cookies.getAll({ url });
      output.push(...found);
    } catch (err) {
      console.warn('[Bridge] Cookie URL query failed:', url, err);
    }
  }
  return output;
}

async function handleCookieImport(requestId, providerId, domains, cookieNames) {
  try {
    const cookies = await collectCookies(domains, cookieNames);
    sendImportResult(requestId, providerId, cookies, null, {});
  } catch (err) {
    sendImportResult(requestId, providerId, [], err.message || 'cookie_fetch_failed', {});
  }
}

async function handleCodexImport(requestId, payload) {
  const localStorageData = {};
  let cookies = [];
  try {
    cookies = await collectCookies(payload.domains || [], payload.cookieNames || []);
  } catch (err) {
    localStorageData.codex_cookie_error = sanitizeBridgeError(err);
  }

  try {
    const usageJson = await fetchCodexUsageSnapshot();
    localStorageData.codex_usage_json = JSON.stringify(usageJson);
    sendImportResult(requestId, 'codex', cookies, undefined, localStorageData);
  } catch (err) {
    const errorMessage = sanitizeBridgeError(err);
    localStorageData.codex_usage_error = errorMessage;
    sendImportResult(
      requestId,
      'codex',
      cookies,
      'codex_usage_fetch_failed',
      localStorageData,
      errorMessage
    );
  }
}

async function fetchCodexUsageSnapshot() {
  let tabId = null;
  let createdTab = false;
  try {
    const existingTabs = await chrome.tabs.query({ url: 'https://chatgpt.com/*' });
    if (existingTabs.length > 0) {
      tabId = existingTabs[0].id;
    } else {
      const tab = await chrome.tabs.create({
        url: 'https://chatgpt.com/codex/settings/usage',
        active: false
      });
      tabId = tab.id;
      createdTab = true;
      await waitForTabComplete(tabId);
    }

    const results = await chrome.scripting.executeScript({
      target: { tabId },
      world: 'MAIN',
      func: async () => {
        const redact = (text) => String(text || '')
          .replace(/"accessToken"\s*:\s*"[^"]*"/gi, '"accessToken":"[redacted]"')
          .replace(/"access_token"\s*:\s*"[^"]*"/gi, '"access_token":"[redacted]"')
          .replace(/Bearer\s+[A-Za-z0-9._-]+/g, 'Bearer [redacted]');
        const preview = (text) => redact(String(text || '').replace(/\s+/g, ' ').trim()).slice(0, 220);
        const firstString = (values) => {
          for (const value of values) {
            if (typeof value === 'string' && value.trim()) return value.trim();
          }
          return '';
        };
        const accountIdFromSession = (json) => {
          if (!json || typeof json !== 'object') return '';
          const direct = firstString([
            json.accountId,
            json.account_id,
            json.activeAccountId,
            json.active_account_id
          ]);
          if (direct) return direct;
          const accounts = Array.isArray(json.accounts) ? json.accounts : [];
          for (const account of accounts) {
            if (!account || typeof account !== 'object') continue;
            const id = firstString([
              account.id,
              account.account_id,
              account.accountId,
              account.account?.id
            ]);
            if (id) return id;
          }
          return '';
        };
        const readJson = async (url, stage, options = {}) => {
          let response;
          try {
            response = await fetch(url, {
              method: 'GET',
              credentials: 'include',
              ...options,
              headers: {
                Accept: 'application/json',
                ...(options.headers || {})
              }
            });
          } catch (err) {
            throw new Error(`${stage}: network error: ${err && err.message ? err.message : err}`);
          }

          const contentType = response.headers.get('content-type') || '';
          const text = await response.text();
          if (!response.ok) {
            throw new Error(`${stage}: HTTP ${response.status}; content-type=${contentType}; preview=${preview(text)}`);
          }
          if (!contentType.toLowerCase().includes('json')) {
            throw new Error(`${stage}: expected JSON but got ${contentType || 'unknown content-type'}; preview=${preview(text)}`);
          }
          try {
            return JSON.parse(text);
          } catch (err) {
            throw new Error(`${stage}: invalid JSON; content-type=${contentType}; preview=${preview(text)}`);
          }
        };

        const authJson = await readJson('/api/auth/session', 'codex_auth_session');
        const usageHeaders = {};
        const accountId = accountIdFromSession(authJson);
        if (accountId) usageHeaders['ChatGPT-Account-Id'] = accountId;
        const bearer = firstString([authJson.accessToken, authJson.access_token]);
        if (bearer) usageHeaders.Authorization = `Bearer ${bearer}`;

        const usageJson = await readJson('/backend-api/wham/usage', 'codex_usage_snapshot', {
          headers: usageHeaders
        });

        const email = firstString([
          usageJson.email,
          usageJson.account_email,
          authJson.user?.email,
          authJson.account?.email
        ]);
        if (email && !usageJson.email) usageJson.email = email;
        const plan = firstString([
          usageJson.plan_type,
          usageJson.plan,
          authJson.user?.plan_type,
          authJson.account?.plan_type
        ]);
        if (plan && !usageJson.plan_type) usageJson.plan_type = plan;
        return { ok: true, usageJson };
      }
    });

    const result = results && results.length > 0 ? results[0].result : null;
    if (!result) {
      throw new Error('codex_usage_empty_result');
    }
    if (!result.ok) {
      throw new Error(result.errorMessage || 'codex_usage_fetch_failed');
    }
    return result.usageJson;
  } finally {
    if (createdTab && tabId !== null) {
      chrome.tabs.remove(tabId).catch(() => {});
    }
  }
}

function sanitizeBridgeError(error) {
  const message = error && error.message ? error.message : String(error || 'unknown_error');
  return message
    .replace(/"accessToken"\s*:\s*"[^"]*"/gi, '"accessToken":"[redacted]"')
    .replace(/"access_token"\s*:\s*"[^"]*"/gi, '"access_token":"[redacted]"')
    .replace(/Bearer\s+[A-Za-z0-9._-]+/g, 'Bearer [redacted]')
    .slice(0, 500);
}

async function handleLocalStorageImport(requestId, providerId, origin, localStorageKeys) {
  let tabId = null;
  let createdTab = false;
  try {
    const existingTabs = await chrome.tabs.query({ url: origin + '/*' });
    if (existingTabs.length > 0) {
      tabId = existingTabs[0].id;
    } else {
      const tab = await chrome.tabs.create({ url: origin, active: false });
      tabId = tab.id;
      createdTab = true;
      await waitForTabComplete(tabId);
    }

    const results = await chrome.scripting.executeScript({
      target: { tabId },
      func: (keys) => {
        const output = {};
        for (const key of keys) {
          const value = localStorage.getItem(key);
          if (value !== null) output[key] = value;
        }
        return output;
      },
      args: [localStorageKeys]
    });

    const localStorageData = (results && results.length > 0) ? results[0].result : {};
    sendImportResult(requestId, providerId, [], null, localStorageData || {});
  } catch (err) {
    sendImportResult(requestId, providerId, [], err.message || 'localstorage_fetch_failed', {});
  } finally {
    if (createdTab && tabId !== null) {
      chrome.tabs.remove(tabId).catch(() => {});
    }
  }
}

function waitForTabComplete(tabId) {
  return new Promise((resolve, reject) => {
    const timeout = setTimeout(() => {
      chrome.tabs.onUpdated.removeListener(listener);
      reject(new Error('tab_load_timeout'));
    }, 15000);

    function listener(updatedTabId, changeInfo) {
      if (updatedTabId === tabId && changeInfo.status === 'complete') {
        clearTimeout(timeout);
        chrome.tabs.onUpdated.removeListener(listener);
        resolve();
      }
    }

    chrome.tabs.onUpdated.addListener(listener);
  });
}

function sendImportResult(requestId, providerId, cookies, error, localStorageData, errorMessage) {
  const payload = {
    type: BridgeProtocol.message.importResult,
    requestId,
    providerId,
    success: false,
    cookies,
    localStorage: localStorageData || {},
    capturedAtUtc: new Date().toISOString(),
    sourceReason: 'manual_request',
    schemaVersion: 1
  };
  if (error) {
    payload.errorCode = typeof error === 'string' ? error : 'unknown_error';
    payload.errorMessage = errorMessage || payload.errorCode;
  } else {
    payload.success = true;
  }

  lastImportResult = {
    providerId,
    success: payload.success,
    errorMessage: payload.errorMessage || '',
    capturedAt: payload.capturedAtUtc
  };
  chrome.runtime.sendMessage({ type: 'importResult', result: lastImportResult }).catch(() => {});
  sendMessage(payload);
}

chrome.alarms.create('bridge_keepalive', { periodInMinutes: 0.5 });
chrome.alarms.onAlarm.addListener((alarm) => {
  if (alarm.name === 'bridge_keepalive' && (!socket || socket.readyState !== WebSocket.OPEN)) {
    connect();
  }
});

chrome.runtime.onStartup.addListener(() => connect());
chrome.runtime.onInstalled.addListener(() => connect());

chrome.runtime.onMessage.addListener((message, sender, sendResponse) => {
  if (message.type === 'getState') {
    sendResponse({ connectionState, profileInstanceId, profileAlias, lastImportResult });
    return true;
  }
  if (message.type === 'setAlias') {
    saveProfileAlias(message.alias).then(() => {
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
  return false;
});

(async function init() {
  await loadProfile();
  await loadRuntimeConfig();
  connect();
})();
