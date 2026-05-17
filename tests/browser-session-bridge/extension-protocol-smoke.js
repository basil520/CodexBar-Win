// Extension Protocol Smoke Test
// Run with: node tests/browser-session-bridge/extension-protocol-smoke.js

const assert = require('assert');
const fs = require('fs');
const path = require('path');

function buildRegisterClient() {
  return {
    type: 'register_client',
    protocolVersion: 1,
    extensionId: 'cnanalhpjiclhljkpnlbgiaclpbncidk',
    browserFamily: 'chrome',
    browserVersion: '127.0.0.0',
    extensionBuild: '2026.05.17',
    profileInstanceId: 'uuid-test-001',
    profileAlias: 'Default',
    incognito: false,
    capabilities: {
      cookies: true,
      localStorage: true,
      codexUsageSnapshot: true
    }
  };
}

function buildImportResult() {
  return {
    type: 'import_result',
    requestId: 'req-001',
    providerId: 'cursor',
    success: true,
    cookies: [
      {
        name: 'WorkosCursorSessionToken',
        value: 'test-token',
        domain: '.cursor.com',
        path: '/',
        sameSite: 'lax',
        storeId: '0',
        secure: true,
        httpOnly: true,
        hostOnly: false,
        session: false,
        expirationDate: 1780000000,
        partitionKey: ''
      }
    ],
    localStorage: {},
    capturedAtUtc: new Date().toISOString(),
    sourceReason: 'manual_request',
    schemaVersion: 1
  };
}

function buildRequestImport() {
  return {
    type: 'request_import',
    requestId: 'req-001',
    providerId: 'cursor',
    materialKind: 'cookies',
    domains: ['cursor.com'],
    cookieNames: ['WorkosCursorSessionToken'],
    origin: '',
    localStorageKeys: []
  };
}

function validateRegisterClient(msg) {
  assert.strictEqual(msg.type, 'register_client');
  assert.strictEqual(typeof msg.protocolVersion, 'number');
  assert.strictEqual(typeof msg.extensionId, 'string');
  assert.strictEqual(typeof msg.browserFamily, 'string');
  assert.strictEqual(typeof msg.browserVersion, 'string');
  assert.strictEqual(typeof msg.extensionBuild, 'string');
  assert.strictEqual(typeof msg.profileInstanceId, 'string');
  assert.strictEqual(typeof msg.profileAlias, 'string');
  assert.strictEqual(typeof msg.incognito, 'boolean');
  assert.strictEqual(typeof msg.capabilities.cookies, 'boolean');
  assert.strictEqual(typeof msg.capabilities.localStorage, 'boolean');
  assert.strictEqual(typeof msg.capabilities.codexUsageSnapshot, 'boolean');
}

function validateImportResult(msg) {
  assert.strictEqual(msg.type, 'import_result');
  assert.strictEqual(typeof msg.requestId, 'string');
  assert.strictEqual(typeof msg.providerId, 'string');
  assert.strictEqual(typeof msg.success, 'boolean');
  assert.ok(Array.isArray(msg.cookies));
  assert.strictEqual(typeof msg.localStorage, 'object');
  assert.strictEqual(typeof msg.capturedAtUtc, 'string');
  assert.strictEqual(typeof msg.sourceReason, 'string');
  assert.strictEqual(typeof msg.schemaVersion, 'number');
}

function validateRequestImport(msg) {
  assert.strictEqual(msg.type, 'request_import');
  assert.strictEqual(typeof msg.requestId, 'string');
  assert.strictEqual(typeof msg.providerId, 'string');
  assert.ok(['cookies', 'localStorage', 'hybrid'].includes(msg.materialKind));
  assert.ok(Array.isArray(msg.domains));
  assert.ok(Array.isArray(msg.cookieNames));
}

const reg = buildRegisterClient();
validateRegisterClient(reg);
const imp = buildImportResult();
validateImportResult(imp);
const req = buildRequestImport();
validateRequestImport(req);
validateImportResult(JSON.parse(JSON.stringify(imp)));

const serviceWorker = fs.readFileSync(
  path.join(__dirname, '..', '..', 'resources', 'browser-session-bridge', 'service_worker.js'),
  'utf8'
);
assert.ok(serviceWorker.includes('fetchCodexUsageSnapshot'));
assert.ok(serviceWorker.includes('chrome.scripting.executeScript'));
assert.ok(serviceWorker.includes('cookieQueryUrlsForDomain'));
assert.ok(serviceWorker.includes('chrome.cookies.getAll({ url'));
assert.ok(serviceWorker.includes('codexUsageSnapshot'));
assert.ok(serviceWorker.includes('extensionBuild'));
assert.ok(serviceWorker.includes('/backend-api/wham/usage'));
assert.ok(serviceWorker.includes("credentials: 'include'"));
assert.ok(serviceWorker.includes('codex_usage_json'));
assert.ok(serviceWorker.includes('success: false') || serviceWorker.includes('false,'));
assert.ok(!serviceWorker.includes("sendImportResult(requestId, 'codex', cookies, null"));
assert.ok(!serviceWorker.includes('codex_access_token'));

console.log('All extension protocol smoke tests passed.');
