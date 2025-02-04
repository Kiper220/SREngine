//
// Created by Monika on 18.12.2022.
//

#ifndef SRENGINE_SCENEBUILDER_H
#define SRENGINE_SCENEBUILDER_H

#include <Utils/Common/NonCopyable.h>

namespace SR_UTILS_NS {
    class Component;
}

namespace SR_WORLD_NS {
    class Scene;

    class SceneBuilder : public SR_UTILS_NS::NonCopyable {
        using Super = SR_UTILS_NS::NonCopyable;
    public:
        explicit SceneBuilder(Scene* pScene);
        ~SceneBuilder() override = default;

    public:
        void Build(bool isPaused);
        void Update(float_t dt);
        void FixedUpdate();

        void SetDirty();

    private:
        void Initialize(bool isPaused);

    private:
        std::vector<SR_UTILS_NS::Component*> m_updatableComponents;
        bool m_dirty = false;
        uint64_t m_rootHash = 0;
        Scene* m_scene = nullptr;

    };
}

#endif //SRENGINE_SCENEBUILDER_H
