//
// Created by Nikita on 17.11.2020.
//

#include <Utils/ResourceManager/IResource.h>
#include <Utils/ResourceManager/ResourceManager.h>

namespace SR_UTILS_NS {
    IResource::IResource(uint64_t hashName)
        : Super()
        , m_resourceHashName(hashName)
        , m_lifetime(ResourceManager::ResourceLifeTime)
    { }

    IResource::IResource(uint64_t hashName, bool autoRemove)
        : Super()
        , m_resourceHashName(hashName)
        , m_lifetime(ResourceManager::ResourceLifeTime)
        , m_autoRemove(autoRemove)
    { }

    bool IResource::Reload() {
        SR_LOCK_GUARD

        SR_LOG("IResource::Reload() : reloading \"" + std::string(GetResourceId()) + "\" resource...");

        m_loadState = LoadState::Reloading;

        Unload();

        if (!Load()) {
            m_loadState = LoadState::Error;
            return false;
        }

        m_loadState = LoadState::Loaded;

        UpdateResources();
        OnReloadDone();

        return true;
    }

    bool IResource::ForceDestroy() {
        if (m_force || IsDestroyed()) {
            SR_ERROR("IResource::ForceDestroy() : resource is already destroyed!");
            return false;
        }

        m_force = true;

        return Destroy();
    }

    std::string_view IResource::GetResourceName() const {
        return ResourceManager::Instance().GetTypeName(m_resourceHashName);
    }

    void IResource::SetId(uint64_t hashId, bool autoRegister) {
        if (m_resourceHashId == 0) {
            m_resourceHashId = hashId;

            SRAssert(m_resourceHashPath != 0);

            if (autoRegister) {
                ResourceManager::Instance().RegisterResource(this);
            }
        }
        else {
            SRHalt("Double set resource id!");
        }
    }

    void IResource::SetId(const std::string &id, bool autoRegister) {
        SRAssert2(!id.empty(), "Invalid id!");

        if (m_resourceHashId == 0) {
            auto&& resourcesManager = ResourceManager::Instance();

            /// обязательно присваиваем до инициализации пути
            m_resourceHashId = resourcesManager.RegisterResourceId(id);

            SRAssert(m_resourceHashPath == 0);

            auto&& path = InitializeResourcePath();
            m_resourceHashPath = resourcesManager.RegisterResourcePath(path);

            if (autoRegister) {
                resourcesManager.RegisterResource(this);
            }
        }
        else {
            SRHalt("Double set resource id!");
        }
    }

    IResource::RemoveUPResult IResource::RemoveUsePoint() {
        RemoveUPResult result;

        /// тут нужно делать синхронно, иначе может произойти deadlock
        ResourceManager::Instance().Execute([this, &result]() {
            m_mutex.lock();

            if (m_countUses == 0) {
                SRHalt("Count use points is zero!");
                result = RemoveUPResult::Error;
                return;
            }

            --m_countUses;

            if (m_countUses == 0 && m_autoRemove && !IsDestroyed()) {
                if (IsRegistered()) {
                    Destroy();
                    result = RemoveUPResult::Destroy;
                    m_mutex.unlock();
                    return;
                }
                else {
                    /// так и не зарегистрировали ресурс
                    m_mutex.unlock();
                    delete this;
                    result = RemoveUPResult::Delete;
                    return;
                }
            }

            result = RemoveUPResult::Success;

            m_mutex.unlock();
        });

        return result;
    }

    void IResource::AddUsePoint() {
        SR_LOCK_GUARD

        SRAssert(m_countUses != SR_UINT16_MAX);

        if (m_isRegistered && m_countUses == 0 && m_isDestroyed) {
            SRHalt("IResource::AddUsePoint() : potential multi threading error!");
        }

        ++m_countUses;
    }

    uint16_t IResource::GetCountUses() const noexcept {
        return m_countUses;
    }

    bool IResource::IsDestroyed() const noexcept {
        return m_isDestroyed;
    }

    IResource *IResource::CopyResource(IResource *destination) const {
        SR_LOCK_GUARD

        destination->m_autoRemove = m_autoRemove;
        /// destination->m_lifetime = m_lifetime;
        destination->m_resourceHashPath = m_resourceHashPath;
        destination->m_loadState.store(m_loadState);

        destination->SetId(m_resourceHashId, true /** auto register */);

        destination->SetReadOnly(m_readOnly);

        return destination;
    }

    bool IResource::Destroy() {
        SR_LOCK_GUARD

        SRAssert(!IsDestroyed());
        m_isDestroyed = true;

        ResourceManager::Instance().Destroy(this);

        return true;
    }

    bool IResource::Kill() {
        if (GetCountUses() == 0) {
            if (!IsDestroyed()) {
                Destroy();
            }

            m_lifetime = 0.f;

            return true;
        }

        return false;
    }

    uint64_t IResource::GetFileHash() const {
        auto&& path = Path(GetResourcePath());

        if (!path.IsAbs()) {
            path = GetAssociatedPath().Concat(path);
        }

        if (path.Exists(Path::Type::File)) {
            if (auto&& hash = path.GetFileHash(); hash != SR_UINT64_MAX) {
                return hash;
            }
        }

        SRHaltOnce("IResource::GetFileHash() : failed to get resource hash! \n\tResource id: " + std::string(GetResourceId()) +
            "\n\tResource path: " + path.ToString());

        return 0;
    }

    void IResource::SetResourceHash(uint64_t hash) {
        m_resourceHash = hash;
    }

    const std::string& IResource::GetResourceId() const {
        return SR_UTILS_NS::ResourceManager::Instance().GetResourceId(m_resourceHashId);
    }

    const Path& IResource::GetResourcePath() const {
        return SR_UTILS_NS::ResourceManager::Instance().GetResourcePath(m_resourceHashPath);
    }

    Path IResource::InitializeResourcePath() const {
        return SR_UTILS_NS::Path(GetResourceId(), false /** fast */);
    }

    Path IResource::GetAssociatedPath() const {
        return SR_UTILS_NS::ResourceManager::Instance().GetResPath();
    }

    bool IResource::TryExecute(const SR_HTYPES_NS::Function<bool()>& fun, bool def) const {
        if (m_mutex.try_lock()) {
            const bool result = fun();
            m_mutex.unlock();
            return result;
        }

        return def;
    }

    bool IResource::Execute(const SR_HTYPES_NS::Function<bool()>& fun) const {
        SR_LOCK_GUARD
        return fun();
    }

    void IResource::ReviveResource() {
        SR_LOCK_GUARD

        SRAssert(m_isDestroyed && m_isRegistered);

        m_isDestroyed = false;

        UpdateResourceLifeTime();
    }

    void IResource::UpdateResourceLifeTime() {
        m_lifetime = ResourceManager::ResourceLifeTime;
    }
}