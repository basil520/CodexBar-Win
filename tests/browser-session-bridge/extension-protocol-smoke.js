// Extension Protocol Smoke Test
// Verifies JS-side message structure matches the C++ BridgeProtocol schema.
// Run with: node tests/browser-session-bridge/extension-protocol-smoke.js

const assert = require('assert');

const BRIDGE_PROTOCOL_VERSION = 1;

function buildRegisterClient() {
  return {
    type: 'RegisterClient',
    payload: {
      protocolVersion: BRIDGE_PROTOCOL_VERSION,
      extensionId: 'test-extension-id',
      browserFamily: 'chrome',
      browserVersion: '136.0.0.0',
      profileInstanceId: 'uuid-test-001',
      profileAlias: 'Test Chrome',
      supportsCookies: true,
      supportsLocalStorage: false,
      incognito: false
    }
  };
}

function buildImportResult() {
  return {
    type: 'ImportResult',
    payload: {
      requestId: 'req-001',
      providerId: 'cursor',
      success: true,
      cookies: [
        {
          name: 'WorkosCursorSessionToken',
          value: 'test-token',
          domain: '.cursor.com',
          path: '/',
          sameSite: 'Lax',
          storeId: '0',
          secure: true,
          httpOnly: true,
          hostOnly: false,
          session: false,
          expirationDateUtc: '2026-05-16T12:00:00.000Z',
          partitionKey: ''
        }
      ],
      localStorage: {},
      capturedAtUtc: new Date().toISOString(),
      sourceReason: 'manual_request',
      schemaVersion: 1
    }
  };
}

function buildRequestImport() {
  return {
    type: 'RequestImport',
    payload: {
      requestId: 'req-001',
      providerId: 'cursor',
      materialKind: 'Cookies',
      domains: ['cursor.com'],
      cookieNames: ['WorkosCursorSessionToken'],
      localStorageOrigin: '',
      localStorageKeys: []
    }
  };
}

function validateRegisterClient(msg) {
  assert.strictEqual(msg.type, 'RegisterClient', 'type must be RegisterClient');
  const p = msg.payload;
  assert.strictEqual(typeof p.protocolVersion, 'number', 'protocolVersion must be number');
  assert.strictEqual(typeof p.extensionId, 'string', 'extensionId must be string');
  assert.strictEqual(typeof p.browserFamily, 'string', 'browserFamily must be string');
  assert.strictEqual(typeof p.browserVersion, 'string', 'browserVersion must be string');
  assert.strictEqual(typeof p.profileInstanceId, 'string', 'profileInstanceId must be string');
  assert.strictEqual(typeof p.profileAlias, 'string', 'profileAlias must be string');
  assert.strictEqual(typeof p.supportsCookies, 'boolean', 'supportsCookies must be boolean');
  assert.strictEqual(typeof p.supportsLocalStorage, 'boolean', 'supportsLocalStorage must be boolean');
  assert.strictEqual(typeof p.incognito, 'boolean', 'incognito must be boolean');
}

function validateImportResult(msg) {
  assert.strictEqual(msg.type, 'ImportResult', 'type must be ImportResult');
  const p = msg.payload;
  assert.strictEqual(typeof p.requestId, 'string', 'requestId must be string');
  assert.strictEqual(typeof p.providerId, 'string', 'providerId must be string');
  assert.strictEqual(typeof p.success, 'boolean', 'success must be boolean');
  assert.ok(Array.isArray(p.cookies), 'cookies must be array');
  assert.strictEqual(typeof p.localStorage, 'object', 'localStorage must be object');
  assert.strictEqual(typeof p.capturedAtUtc, 'string', 'capturedAtUtc must be string');
  assert.strictEqual(typeof p.sourceReason, 'string', 'sourceReason must be string');
  assert.strictEqual(typeof p.schemaVersion, 'number', 'schemaVersion must be number');

  if (p.cookies.length > 0) {
    const c = p.cookies[0];
    assert.strictEqual(typeof c.name, 'string', 'cookie.name must be string');
    assert.strictEqual(typeof c.value, 'string', 'cookie.value must be string');
    assert.strictEqual(typeof c.domain, 'string', 'cookie.domain must be string');
    assert.strictEqual(typeof c.path, 'string', 'cookie.path must be string');
  }
}

function validateRequestImport(msg) {
  assert.strictEqual(msg.type, 'RequestImport', 'type must be RequestImport');
  const p = msg.payload;
  assert.strictEqual(typeof p.requestId, 'string', 'requestId must be string');
  assert.strictEqual(typeof p.providerId, 'string', 'providerId must be string');
  assert.strictEqual(typeof p.materialKind, 'string', 'materialKind must be string');
  assert.ok(Array.isArray(p.domains), 'domains must be array');
  assert.ok(Array.isArray(p.cookieNames), 'cookieNames must be array');
}

function runTests() {
  console.log('Running extension protocol smoke tests...\n');

  const reg = buildRegisterClient();
  validateRegisterClient(reg);
  console.log('  [PASS] RegisterClient schema');

  const imp = buildImportResult();
  validateImportResult(imp);
  console.log('  [PASS] ImportResult schema');

  const req = buildRequestImport();
  validateRequestImport(req);
  console.log('  [PASS] RequestImport schema');

  // Round-trip serialization test
  const serialized = JSON.stringify(imp);
  const deserialized = JSON.parse(serialized);
  validateImportResult(deserialized);
  assert.strictEqual(deserialized.payload.cookies[0].name, 'WorkosCursorSessionToken');
  console.log('  [PASS] ImportResult JSON round-trip');

  console.log('\nAll smoke tests passed.');
}

runTests();
