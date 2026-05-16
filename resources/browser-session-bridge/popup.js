document.addEventListener('DOMContentLoaded', async () => {
  const statusDot = document.getElementById('status-dot');
  const profileIdEl = document.getElementById('profile-id');
  const aliasInput = document.getElementById('alias-input');
  const saveAliasBtn = document.getElementById('save-alias-btn');
  const reconnectBtn = document.getElementById('reconnect-btn');
  const lastImportEl = document.getElementById('last-import');
  const lastImportStatus = document.getElementById('last-import-status');

  async function refreshState() {
    try {
      const state = await chrome.runtime.sendMessage({ type: 'getState' });
      statusDot.className = 'status ' + (state.connectionState || 'disconnected');
      profileIdEl.textContent = state.profileInstanceId || '—';
      aliasInput.value = state.profileAlias || '';

      if (state.lastImportResult) {
        lastImportEl.style.display = '';
        const r = state.lastImportResult;
        const time = new Date(r.capturedAt).toLocaleTimeString();
        if (r.success) {
          lastImportStatus.innerHTML = `<span class="ok">OK</span> (${time})`;
        } else {
          lastImportStatus.innerHTML = `<span class="fail">Failed</span> (${time})`;
        }
      } else {
        lastImportEl.style.display = 'none';
      }
    } catch (e) {
      statusDot.className = 'status disconnected';
    }
  }

  saveAliasBtn.addEventListener('click', async () => {
    saveAliasBtn.disabled = true;
    try {
      await chrome.runtime.sendMessage({ type: 'setAlias', alias: aliasInput.value.trim() });
    } catch (e) {
      // ignore
    }
    saveAliasBtn.disabled = false;
  });

  reconnectBtn.addEventListener('click', async () => {
    reconnectBtn.disabled = true;
    try {
      await chrome.runtime.sendMessage({ type: 'reconnect' });
    } catch (e) {
      // ignore
    }
    setTimeout(() => { reconnectBtn.disabled = false; }, 1000);
  });

  // Refresh on popup open
  await refreshState();

  // Listen for state updates from service worker
  chrome.runtime.onMessage.addListener((msg) => {
    if (msg.type === 'connectionStateChanged' || msg.type === 'importResult') {
      refreshState();
    }
  });
});
