#include "ProviderCredentialStore.h"

#ifdef Q_OS_MACOS
#include "MacOSCredentialStore.h"
#elif defined(Q_OS_WIN)
#include "WindowsCredentialStore.h"
#endif

#include <mutex>

namespace {

class NativeCredentialBackend : public ProviderCredentialBackend {
public:
    bool write(const QString& target, const QString& username, const QByteArray& secret) override {
#ifdef Q_OS_MACOS
        return MacOSCredentialStore::write(target, username, secret);
#elif defined(Q_OS_WIN)
        return WindowsCredentialStore::write(target, username, secret);
#else
        Q_UNUSED(target)
        Q_UNUSED(username)
        Q_UNUSED(secret)
        return false;
#endif
    }

    std::optional<QByteArray> read(const QString& target) override {
#ifdef Q_OS_MACOS
        return MacOSCredentialStore::read(target);
#elif defined(Q_OS_WIN)
        return WindowsCredentialStore::read(target);
#else
        Q_UNUSED(target)
        return std::nullopt;
#endif
    }

    bool remove(const QString& target) override {
#ifdef Q_OS_MACOS
        return MacOSCredentialStore::remove(target);
#elif defined(Q_OS_WIN)
        return WindowsCredentialStore::remove(target);
#else
        Q_UNUSED(target)
        return false;
#endif
    }

    bool exists(const QString& target) override {
#ifdef Q_OS_MACOS
        return MacOSCredentialStore::exists(target);
#elif defined(Q_OS_WIN)
        return WindowsCredentialStore::exists(target);
#else
        Q_UNUSED(target)
        return false;
#endif
    }
};

std::shared_ptr<ProviderCredentialBackend>& backend()
{
    static std::shared_ptr<ProviderCredentialBackend> instance =
        std::make_shared<NativeCredentialBackend>();
    return instance;
}

std::mutex& backendMutex()
{
    static std::mutex mutex;
    return mutex;
}

} // namespace

// --- Sharding helpers ---

QString ProviderCredentialStore::shardTarget(const QString& base, int index)
{
    return base + QStringLiteral("__part_") + QString::number(index);
}

QString ProviderCredentialStore::shardCountTarget(const QString& base)
{
    return base + QStringLiteral("__shard_count");
}

bool ProviderCredentialStore::cleanupShards(ProviderCredentialBackend* b,
                                            const QString& base)
{
    const auto countTarget = shardCountTarget(base);
    const auto countData = b->read(countTarget);
    if (countData.has_value()) {
        bool ok = true;
        const int numShards = countData.value().toInt();
        for (int i = 0; i < numShards; ++i) {
            b->remove(shardTarget(base, i));
        }
        ok &= b->remove(countTarget);
        return ok;
    }
    return true;
}

bool ProviderCredentialStore::writeSharded(ProviderCredentialBackend* b,
                                           const QString& base,
                                           const QString& username,
                                           const QByteArray& data)
{
    // Clean up any existing shards or base-target data first
    cleanupShards(b, base);
    b->remove(base);

    const int numShards = (data.size() + MAX_SHARD_SIZE - 1) / MAX_SHARD_SIZE;

    for (int i = 0; i < numShards; ++i) {
        const QByteArray shard = data.mid(i * MAX_SHARD_SIZE, MAX_SHARD_SIZE);
        if (!b->write(shardTarget(base, i), username, shard)) {
            // Rollback: remove already-written shards
            for (int j = 0; j < i; ++j) {
                b->remove(shardTarget(base, j));
            }
            return false;
        }
    }

    // Write shard count marker
    if (!b->write(shardCountTarget(base), username, QByteArray::number(numShards))) {
        // Rollback: remove all shards
        for (int i = 0; i < numShards; ++i) {
            b->remove(shardTarget(base, i));
        }
        return false;
    }

    return true;
}

std::optional<QByteArray> ProviderCredentialStore::readSharded(ProviderCredentialBackend* b,
                                                               const QString& base)
{
    const auto countData = b->read(shardCountTarget(base));
    if (!countData.has_value()) {
        return std::nullopt;
    }

    const int numShards = countData.value().toInt();
    if (numShards <= 0) {
        return std::nullopt;
    }

    QByteArray result;
    result.reserve(numShards * MAX_SHARD_SIZE);

    for (int i = 0; i < numShards; ++i) {
        const auto shard = b->read(shardTarget(base, i));
        if (!shard.has_value()) {
            return std::nullopt;
        }
        result += shard.value();
    }

    return result;
}

bool ProviderCredentialStore::removeSharded(ProviderCredentialBackend* b,
                                            const QString& base)
{
    bool removedAny = cleanupShards(b, base);
    removedAny |= b->remove(base);
    return removedAny;
}

// --- Public API ---

bool ProviderCredentialStore::write(const QString& target,
                                    const QString& username,
                                    const QByteArray& secret)
{
    std::lock_guard<std::mutex> lock(backendMutex());
    auto* b = backend().get();

    if (secret.size() <= MAX_SHARD_SIZE) {
        // Small payload: clean up any existing shards first, then write base target
        cleanupShards(b, target);
        return b->write(target, username, secret);
    }

    return writeSharded(b, target, username, secret);
}

std::optional<QByteArray> ProviderCredentialStore::read(const QString& target)
{
    std::lock_guard<std::mutex> lock(backendMutex());
    auto* b = backend().get();

    // Try sharded read first
    auto sharded = readSharded(b, target);
    if (sharded.has_value()) {
        return sharded;
    }

    // Fallback to base target (backward-compatible for non-sharded data)
    return b->read(target);
}

bool ProviderCredentialStore::remove(const QString& target)
{
    std::lock_guard<std::mutex> lock(backendMutex());
    auto* b = backend().get();
    return removeSharded(b, target);
}

bool ProviderCredentialStore::exists(const QString& target)
{
    std::lock_guard<std::mutex> lock(backendMutex());
    auto* b = backend().get();
    return b->exists(target) || b->exists(shardCountTarget(target));
}

void ProviderCredentialStore::setBackendForTesting(std::shared_ptr<ProviderCredentialBackend> testBackend)
{
    std::lock_guard<std::mutex> lock(backendMutex());
    backend() = testBackend ? std::move(testBackend) : std::make_shared<NativeCredentialBackend>();
}

void ProviderCredentialStore::resetBackendForTesting()
{
    std::lock_guard<std::mutex> lock(backendMutex());
    backend() = std::make_shared<NativeCredentialBackend>();
}

bool InMemoryCredentialBackend::write(const QString& target,
                                      const QString& username,
                                      const QByteArray& secret)
{
    Q_UNUSED(username)
    m_values[target] = secret;
    return true;
}

std::optional<QByteArray> InMemoryCredentialBackend::read(const QString& target)
{
    auto it = m_values.constFind(target);
    if (it == m_values.constEnd()) return std::nullopt;
    return it.value();
}

bool InMemoryCredentialBackend::remove(const QString& target)
{
    const bool existed = m_values.contains(target);
    m_values.remove(target);
    return existed;
}

bool InMemoryCredentialBackend::exists(const QString& target)
{
    return m_values.contains(target);
}
