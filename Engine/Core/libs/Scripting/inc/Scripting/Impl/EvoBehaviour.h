//
// Created by Monika on 24.05.2022.
//

#ifndef SRENGINE_EVOBEHAVIOUR_H
#define SRENGINE_EVOBEHAVIOUR_H

#include <Scripting/Base/Behaviour.h>

namespace SR_SCRIPTING_NS {
    typedef void(*CollisionFnPtr)(const SR_UTILS_NS::CollisionData& data);

    class EvoBehaviour : public SR_SCRIPTING_NS::Behaviour {
        using Properties = std::vector<std::string>;
    private:
        ~EvoBehaviour() override;

    public:
        Properties GetProperties() const override;
        std::any GetProperty(const std::string& id) const override;
        void SetProperty(const std::string& id, const std::any& val) override;

    protected:
        void Awake() override;
        void OnEnable() override;
        void OnDisable() override;
        void Start() override;
        void Update(float_t dt) override;
        void FixedUpdate() override;

        void OnCollisionEnter(const SR_UTILS_NS::CollisionData& data) override;
        void OnCollisionStay(const SR_UTILS_NS::CollisionData& data) override;
        void OnCollisionExit(const SR_UTILS_NS::CollisionData& data) override;
        void OnTriggerEnter(const SR_UTILS_NS::CollisionData& data) override;
        void OnTriggerExit(const SR_UTILS_NS::CollisionData& data) override;
        void OnTriggerStay(const SR_UTILS_NS::CollisionData& data) override;

        bool Load() override;
        bool Unload() override;
        SR_NODISCARD uint64_t GetFileHash() const override;

        void OnDestroy() override;

        void OnAttached() override;

    private:
        void InitHooks();
        void DeInitHooks();
        void SetGameObject();
        void DestroyScript();

        void OnTransformSet() override;

        SR_HTYPES_NS::DataStorage Stash() override;
        void PopStash(const SR_HTYPES_NS::DataStorage &data) override;

    private:
        EvoScript::Script* m_script = nullptr;

        EvoScript::Typedefs::AwakeFnPtr m_awake = nullptr;
        EvoScript::Typedefs::OnEnableFnPtr m_onEnable = nullptr;
        EvoScript::Typedefs::OnDisableFnPtr m_onDisable = nullptr;
        EvoScript::Typedefs::StartFnPtr m_start = nullptr;
        EvoScript::Typedefs::UpdateFnPtr m_update = nullptr;
        EvoScript::Typedefs::FixedUpdateFnPtr m_fixedUpdate = nullptr;

        CollisionFnPtr m_collisionEnter = nullptr;
        CollisionFnPtr m_collisionStay = nullptr;
        CollisionFnPtr m_collisionExit = nullptr;
        CollisionFnPtr m_triggerEnter = nullptr;
        CollisionFnPtr m_triggerStay = nullptr;
        CollisionFnPtr m_triggerExit = nullptr;

        EvoScript::Typedefs::InitBehaviourFnPtr m_initBehaviour = nullptr;
        EvoScript::Typedefs::ReleaseBehaviourFnPtr m_releaseBehaviour = nullptr;
        EvoScript::Typedefs::GetPropertiesFnPtr m_getProperties = nullptr;
        EvoScript::Typedefs::GetPropertyFnPtr m_getProperty = nullptr;
        EvoScript::Typedefs::SetPropertyFnPtr m_setProperty = nullptr;

    };
}

#endif //SRENGINE_EVOBEHAVIOUR_H
