// Generic localStorage probe used by Browser Session Bridge fallback tooling.
(function() {
  const keys = Array.isArray(globalThis.codexBarXLocalStorageKeys)
    ? globalThis.codexBarXLocalStorageKeys
    : [];
  const result = {};
  for (const key of keys) {
    const value = localStorage.getItem(key);
    if (value !== null) result[key] = value;
  }
  return result;
})();
