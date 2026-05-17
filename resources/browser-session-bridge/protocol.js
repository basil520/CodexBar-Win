// Shared Bridge protocol constants for the extension runtime.
const BridgeProtocol = Object.freeze({
  version: 1,
  extensionId: 'cnanalhpjiclhljkpnlbgiaclpbncidk',
  message: Object.freeze({
    registerClient: 'register_client',
    registerAck: 'register_ack',
    requestImport: 'request_import',
    importResult: 'import_result',
    sessionDirty: 'session_dirty',
    ping: 'ping',
    pong: 'pong',
    error: 'error'
  }),
  material: Object.freeze({
    cookies: 'cookies',
    localStorage: 'localStorage',
    hybrid: 'hybrid'
  })
});
