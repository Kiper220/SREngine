//
// Created by Monika on 24.06.2022.
//

#include <Utils/ECS/ComponentManager.h>
#include <Utils/ECS/Migration.h>

namespace SR_UTILS_NS {
    Component* ComponentManager::CreateComponentOfName(const std::string &name) {
        SR_SCOPED_LOCK

        if (m_ids.count(name) == 0) {
            SR_ERROR("ComponentManager::CreateComponentOfName() : component \"" + name + "\" not found!");
            return nullptr;
        }

        return CreateComponentImpl(m_ids.at(name));
    }

    Component* ComponentManager::CreateComponentImpl(size_t id) {
        if (m_meta.count(id) == 0) {
            SR_ERROR("ComponentManager::CreateComponentImpl() : component \"" + std::to_string(id) + "\" not found!");
            return nullptr;
        }

        return m_meta.at(id).constructor();
    }

    Component* ComponentManager::Load(SR_HTYPES_NS::Marshal& marshal) {
        SR_SCOPED_LOCK

        m_lastComponent = marshal.Read<uint64_t>(); /// name
        auto&& enabled = marshal.Read<bool>();      /// enabled
        auto&& version = marshal.Read<uint16_t>();  /// version

        if (version != GetVersionById(m_lastComponent)) {
            auto&& pMetadataIt = m_meta.find(m_lastComponent);
            if (pMetadataIt == m_meta.end()) {
                SRHalt("Unknown component!");
                return nullptr;
            }

            SR_WARN("ComponentManager::Load() : \"" + pMetadataIt->second.name + "\" has different version! Try migrate...");

            if (!Migration::Instance().Migrate(m_lastComponent, marshal, version)) {
                SR_ERROR("ComponentManager::Load() : failed to migrate component!");
                return nullptr;
            }
        }

        if (auto&& pComponent = m_meta.at(m_lastComponent).loader(marshal, &m_context)) {
            pComponent->m_isEnabled = enabled;
            return pComponent;
        }

        return nullptr;
    }

    uint16_t ComponentManager::GetVersion(const Component *pComponent) const {
        SR_SCOPED_LOCK

        return GetVersionById(pComponent->GetComponentHashName());
    }

    uint16_t ComponentManager::GetVersionById(uint64_t id) const {
        SR_SCOPED_LOCK

        auto&& pIt = m_meta.find(id);

        if (pIt == m_meta.end()) {
            return 0;
        }

        return pIt->second.version;
    }

    bool ComponentManager::LoadComponents(const SR_HTYPES_NS::Function<bool(Types::DataStorage & )> &loader) {
        SR_SCOPED_LOCK

        const bool result = loader(m_context);

        m_context.Clear();

        return result;
    }

    std::string ComponentManager::GetLastComponentName() const {
        SR_SCOPED_LOCK

        auto&& pMetadataIt = m_meta.find(m_lastComponent);
        if (pMetadataIt == m_meta.end()) {
            return "\"Unknown component\"";
        }

        return pMetadataIt->second.name;
    }

    std::vector<SR_UTILS_NS::Component*> ComponentManager::LoadComponents(SR_HTYPES_NS::Marshal &marshal) {
        std::vector<SR_UTILS_NS::Component*> components;

        LoadComponents([&](SR_HTYPES_NS::DataStorage& context) -> bool {
            if (m_contextInitializer) {
                m_contextInitializer(context);
            }

            auto&& componentCount = marshal.Read<uint16_t>();
            components.reserve(componentCount);
            SRAssert2(componentCount <= 2048, "While loading the component errors occurred!");

            for (uint32_t i = 0; i < componentCount; ++i) {
                auto&& bytesCount = marshal.Read<uint32_t>();
                auto&& position = marshal.GetPosition();

                /// TODO: use entity id
                SR_MAYBE_UNUSED auto&& compEntityId = marshal.Read<uint64_t>();

                if (auto&& pComponent = Load(marshal)) {
                    components.emplace_back(pComponent);
                }
                else {
                    SR_WARN("ComponentManager::LoadComponents() : failed to load \"" + GetLastComponentName() + "\" component!");
                }

                const uint64_t readBytes = marshal.GetPosition() - position;
                const uint64_t lostBytes = static_cast<uint64_t>(bytesCount) - readBytes;

                if (lostBytes > 0) {
                    SR_WARN("ComponentManager::LoadComponents() : bytes were lost when loading the component!\n\tBytes count: " + std::to_string(lostBytes));
                    if (lostBytes >= UINT16_MAX) {
                        SRHalt("Something went wrong!");
                        return false;
                    }
                    marshal.Skip(lostBytes);
                }
            }

            return true;
        });

        return std::move(components);
    }

    void ComponentManager::SetContextInitializer(const ComponentManager::ContextInitializerFn &fn) {
        m_contextInitializer = fn;
    }
}