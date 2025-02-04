//
// Created by Monika on 28.07.2022.
//

#include <Physics/PhysicsScene.h>

#include <Utils/Math/Mathematics.h>
#include <Utils/World/Scene.h>

#include <Physics/PhysicsWorld.h>
#include <Physics/LibraryImpl.h>

namespace SR_PHYSICS_NS {
    PhysicsScene::PhysicsScene(const ScenePtr& scene)
        : Super(this)
        , m_scene(scene)
    { }

    PhysicsScene::~PhysicsScene() {
        SR_SAFE_DELETE_PTR(m_2DWorld);
        SR_SAFE_DELETE_PTR(m_3DWorld);

        m_library2D = nullptr;
        m_library3D = nullptr;
    }

    bool PhysicsScene::Init() {
        SR_INFO("PhysicsScene::Init() : initializing the physics scene...");

        if (!(m_library2D = SR_PHYSICS_NS::PhysicsLibrary::Instance().GetActiveLibrary(Space::Space2D))) {
            SR_ERROR("PhysicsScene::Init() : failed to get physics 2D library!");
            return false;
        }

        if (!(m_library3D = SR_PHYSICS_NS::PhysicsLibrary::Instance().GetActiveLibrary(Space::Space3D))) {
            SR_ERROR("PhysicsScene::Init() : failed to get physics 3D library!");
            return false;
        }

        if (!(m_2DWorld = m_library2D->CreatePhysicsWorld(Space::Space2D))) {
            SR_ERROR("PhysicsScene::Init() : failed to create 2d world!");
            return false;
        }

        if (!(m_3DWorld = m_library3D->CreatePhysicsWorld(Space::Space3D))) {
            SR_ERROR("PhysicsScene::Init() : failed to create 3d world!");
            return false;
        }

        if (m_scene.RecursiveLockIfValid()) {
            auto&& dataStorage = m_scene->GetDataStorage();

            if (dataStorage.GetValueDef<Ptr>(Ptr())) {
                SR_ERROR("PhysicsScene::Init() : render scene is already exists!");
                m_scene.Unlock();
                return false;
            }

            dataStorage.SetValue<Ptr>(GetThis());

            m_scene.Unlock();
        }
        else {
            SR_ERROR("PhysicsScene::Init() : scene is invalid!");
        }

        if (!CreateDynamicWorld()) {
            SR_ERROR("PhysicsScene::Init() : failed to create dynamic world!");
            return false;
        }

        m_2DWorld->StepSimulation(1.f / 60.f);
        m_3DWorld->StepSimulation(1.f / 60.f);

        return true;
    }

    bool PhysicsScene::CreateDynamicWorld() {
        if (!m_2DWorld->Initialize()) {
            SR_ERROR("PhysicsScene::Initialize() : failed to create dynamic world for 2d world!");
            return false;
        }

        if (!m_3DWorld->Initialize()) {
            SR_ERROR("PhysicsScene::Initialize() : failed to create dynamic world for 3d world!");
            return false;
        }

        return true;
    }

    void PhysicsScene::FixedUpdate() {
        if (m_needClearForces) {
            m_2DWorld->ClearForces();
            m_3DWorld->ClearForces();
            m_needClearForces = false;
        }

        m_2DWorld->StepSimulation(1.f / 60.f);
        m_3DWorld->StepSimulation(1.f / 60.f);

        m_2DWorld->Synchronize();
        m_3DWorld->Synchronize();
    }

    void PhysicsScene::Register(PhysicsScene::RigidbodyPtr pRigidbody) {
        auto&& type = pRigidbody->GetType();

        if (SR_PHYSICS_UTILS_NS::Is2DShape(type)) {
            m_2DWorld->AddRigidbody(pRigidbody);
        }
        else if (SR_PHYSICS_UTILS_NS::Is3DShape(type)) {
            m_3DWorld->AddRigidbody(pRigidbody);
        }
        else {
            SRHalt("Unknown measurement of rigidbody!");
        }
    }

    void PhysicsScene::Remove(PhysicsScene::RigidbodyPtr pRigidbody) {
        auto&& type = pRigidbody->GetType();

        if (SR_PHYSICS_UTILS_NS::Is2DShape(type)) {
            m_2DWorld->RemoveRigidbody(pRigidbody);
        }
        else if (SR_PHYSICS_UTILS_NS::Is3DShape(type)) {
            m_3DWorld->RemoveRigidbody(pRigidbody);
        }
        else {
            SRHalt("Unknown measurement of rigidbody!");
        }
    }

    void PhysicsScene::ClearForces() {
        m_needClearForces = true;
    }
}