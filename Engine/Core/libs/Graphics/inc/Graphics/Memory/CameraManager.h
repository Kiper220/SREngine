//
// Created by Monika on 05.06.2022.
//

#ifndef SRENGINE_CAMERAMANAGER_H
#define SRENGINE_CAMERAMANAGER_H

#include <Utils/Common/Singleton.h>
#include <Utils/Types/SafePointer.h>

namespace SR_GRAPH_NS {
    class Window;
    class RenderScene;
}

namespace SR_GRAPH_NS::Types {
    class Camera;
}

namespace SR_GRAPH_NS::Memory {
    class CameraManager : public SR_UTILS_NS::Singleton<CameraManager> {
        friend class Types::Camera;
        friend class SR_UTILS_NS::Singleton<CameraManager>;
        using CameraPtr = Types::Camera*;
        using RenderScenePtr = SR_HTYPES_NS::SafePtr<RenderScene>;

        struct CameraInfo {
            bool destroyed = false;
            RenderScenePtr pRenderScene;
        };

    protected:
        ~CameraManager() override = default;

    public:
        void Update();

        void OnWindowResized(Window* pWindow, uint32_t width, uint32_t height);

        SR_NODISCARD uint32_t GetCountCameras() const;
        SR_NODISCARD CameraPtr GetFirstCamera() const;
        SR_NODISCARD std::list<CameraPtr> GetCameras() const;
        SR_NODISCARD CameraPtr GetMainCamera(RenderScene* pRScene);
        SR_NODISCARD std::vector<CameraPtr> GetOffScreenCameras(RenderScene* pRScene);

        SR_NODISCARD bool IsRegistered(const CameraPtr& camera) const;
        SR_NODISCARD bool IsDestroyed(const CameraPtr& camera) const;

    protected:
        void OnSingletonDestroy() override;

    private:
        void RegisterCamera(const CameraPtr& camera);
        void UnRegisterCamera(const CameraPtr& camera);
        void DestroyCamera(const CameraPtr& camera);

    private:
        std::map<CameraPtr, CameraInfo> m_cameras;

    };
}

#endif //SRENGINE_CAMERAMANAGER_H
