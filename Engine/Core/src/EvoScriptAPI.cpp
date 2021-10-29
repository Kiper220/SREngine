//
// Created by Nikita on 11.07.2021.
//

#include <EvoScriptAPI.h>

#include <Engine.h>
#include <Input/InputSystem.h>
#include <Loaders/ObjLoader.h>
#include <EntityComponentSystem/Transform.h>
#include <Types/Skybox.h>

#include <Debug.h>
#include <ResourceManager/ResourceManager.h>
#include <GUI/GUISystem.h>

namespace Framework {
    using namespace Core::GUI;
    using namespace Helper::Math;
    using namespace Helper::Types;
    using namespace Graphics;
    using namespace Graphics::GUI;
    using namespace Physics;

    void API::RegisterEvoScriptClasses(Scripting::EvoCompiler* compiler)  {
        auto generator = compiler->GetGenerator();

        RegisterScene(generator);
        RegisterDebug(generator);
        RegisterEngine(generator);
        RegisterComponent(generator);
        RegisterUtils(generator);
        RegisterMesh(generator);
        RegisterResourceManager(generator);
        RegisterGameObject(generator);
        RegisterCamera(generator);
        RegisterWindow(generator);
        RegisterRender(generator);
        RegisterTransform(generator);
        RegisterInput(generator);
        RegisterSkybox(generator);
        RegisterTexture(generator);
        RegisterMaterial(generator);
        RegisterGUISystem(generator);
        RegisterPostProcessing(generator);
        RegisterISavable(generator);

        generator->Save(Helper::ResourceManager::GetResourcesFolder() + "/Scripts/Libraries/");
    }

    void API::RegisterDebug(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Debug", "Debug", {}, {"string"});
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Log, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Warn, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, System, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Shader, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Error, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Info, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, Script, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, ScriptError, void, ESArg1(const std::string& msg), ESArg1(msg))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Debug, ScriptLog, void, ESArg1(const std::string& msg), ESArg1(msg))
    }

    void API::RegisterEngine(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Engine", "Engine", {
                { "volatile bool",  "m_isCreate",  EvoScript::Private },
                { "volatile bool",  "m_isInit",    EvoScript::Private },
                { "volatile bool",  "m_isRun",     EvoScript::Private },
                { "volatile bool",  "m_isClose",   EvoScript::Private },
                { "volatile bool",  "m_exitEvent", EvoScript::Private },
                { "void*",          "m_compiler",  EvoScript::Private },
                { "Window*",        "m_window",    EvoScript::Private },
                { "void*",          "m_render",    EvoScript::Private },
                { "SafePtr<Scene>", "m_scene",     EvoScript::Private },
                { "void*",          "m_time",      EvoScript::Private },
                { "void*",          "m_physics",   EvoScript::Private },
        }, { "Window.h", "Types/SafePointer.h" });
        ESRegisterStaticMethodArg0(Framework::, EvoScript::Public, generator, Engine, Get, Engine*)
        ESRegisterMethodArg0(Framework::,       EvoScript::Private, generator, Engine, RegisterLibraries, bool)
        ESRegisterStaticMethodArg0(Framework::, EvoScript::Public, generator, Engine, Reload, void)
        ESRegisterMethodArg0(Framework::,       EvoScript::Public, generator, Engine, GetTime, Time*)
        ESRegisterMethodArg0(Framework::,       EvoScript::Public, generator, Engine, GetWindow, Window*)
        ESRegisterMethodArg0(Framework::,       EvoScript::Public, generator, Engine, GetRender, Render*)
        ESRegisterMethodArg0(Framework::,       EvoScript::Public, generator, Engine, GetScene, SafePtr<Scene>)
        ESRegisterMethodArg0(Framework::,       EvoScript::Public, generator, Engine, IsRun, bool)
        ESRegisterMethod(Framework::,           EvoScript::Public, generator, Engine, SetScene, bool, ESArg1(const SafePtr<Scene>& scene), ESArg1(scene))

        generator->RegisterTypedef("Time", "Engine", "void");
        generator->RegisterTypedef("PhysEngine", "Engine", "void");

        generator->AddIncompleteType("Scene", "Engine");
        generator->AddIncompleteType("Render", "Engine");
        generator->AddIncompleteType("Camera", "Engine");
    }

    void API::RegisterScene(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("HierarchyElem", "Scene", {
                { "SafePtr<GameObject>",        "m_reference",   EvoScript::Private },
                { "std::string",                "m_displayName", EvoScript::Private },
                { "std::vector<HierarchyElem>", "m_children",    EvoScript::Private },
        });

        generator->RegisterNewClass("Scene", "Scene", {
                { "SafePtr<Scene>",                   "m_this",                EvoScript::Private },

                { "bool",                             "m_isDestroy",           EvoScript::Private },

                { "std::mutex",                       "m_mutex",               EvoScript::Private },
                { "std::mutex",                       "m_displayListMutex",    EvoScript::Private },
                { "std::mutex",                       "m_selectedMutex",       EvoScript::Private },

                { "std::string",                      "m_name",                EvoScript::Private },
                { "std::set<SafePtr<GameObject>>",    "m_gameObjects",         EvoScript::Private },

                { "std::vector<SafePtr<GameObject>>", "m_selectedGameObjects", EvoScript::Private },

                { "std::vector<SafePtr<GameObject>>", "m_rootObjects",         EvoScript::Private },
                { "std::vector<HierarchyElem>",       "m_displayList",         EvoScript::Private },
                { "bool",                             "m_displayListEnabled",  EvoScript::Private },
        }, { "map", "string", "mutex", "vector", "stdint.h", "set", "GameObject.h", "Types/SafePointer.h" });
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, Destroy, bool)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, Free, bool)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, GetName, std::string)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, GetGameObjects, std::vector<SafePtr<GameObject>>)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, GetRootGameObjects, std::vector<SafePtr<GameObject>>&)
        //ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, GetCountUsesPoints, uint32_t, () const)
        //ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, SetIsChanged, void, (bool))
        //ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, IsChanged, bool, () const)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, GetSelected, SafePtr<GameObject>)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Scene, UnselectAll, void)
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, RemoveSelected, bool, ESArg1(const SafePtr<GameObject>& gm), ESArg1(gm))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, AddSelected, void, ESArg1(const SafePtr<GameObject>& gm), ESArg1(gm))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, Instance, SafePtr<GameObject>, ESArg1(const std::string& name), ESArg1(name))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Scene, FindByComponent,  SafePtr<GameObject>, ESArg1(const std::string& name), ESArg1(name))

        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Scene, New, SafePtr<Scene>, ESArg1(const std::string& name), ESArg1(name))
    }

    void API::RegisterUtils(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("_64byte", "Utils", { { "char", "b[64]", EvoScript::Public } },
                                    { "vector","string", "vector", "stdint.h",  });

        generator->RegisterNewClass("_128byte", "Utils", { { "char", "b[128]", EvoScript::Public } });
        generator->RegisterNewClass("_52byte", "Utils", { { "char", "b[52]", EvoScript::Public } });
        generator->RegisterNewClass("_16byte", "Utils", { { "char", "b[16]", EvoScript::Public } });
    }

    void API::RegisterComponent(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Component", "Component", {
                { "bool",              "m_isSelected",  EvoScript::Protected },
                { "bool",              "m_isActive",    EvoScript::Protected },
                { "bool",              "m_isEnabled",   EvoScript::Protected },
                { "const std::string", "m_name",        EvoScript::Protected },
                { "GameObject*",       "m_parent",      EvoScript::Protected },
        }, { "Math/Vector3.h", "string" });
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, GetComponentName, std::string)

        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, BaseComponent, Component*)
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Component, SetParent, void, ESArg1(GameObject* gm), ESArg1(gm))
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, GetParent, GameObject*)

        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Public, generator, Component, DrawOnInspector, bool)
        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Public, generator, Component, GetBarycenter, Vector3)
        ESRegisterMethodVirtual(Helper::, EvoScript::Public, generator, Component, OnRotate, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethodVirtual(Helper::, EvoScript::Public, generator, Component, OnMove, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethodVirtual(Helper::, EvoScript::Public, generator, Component, OnScaled, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethodVirtual(Helper::, EvoScript::Public, generator, Component, OnSelected, void, ESArg1(bool v), ESArg1(v))
        ESRegisterMethodVirtual(Helper::, EvoScript::Public, generator, Component, OnReady, void, ESArg1(bool v), ESArg1(v))
        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Public, generator, Component, OnAttachComponent, void)

        ESRegisterMethod(Helper::, EvoScript::Public, generator, Component, SetActive, void, ESArg1(bool v), ESArg1(v))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Component, SetEnabled, void, ESArg1(bool v), ESArg1(v))

        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, IsActive, bool)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, IsSelected, bool)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Component, IsReady, bool)

        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Protected, generator, Component, OnRemoveComponent, void)
        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Protected, generator, Component, OnDestroyGameObject, void)

        generator->AddIncompleteType("GameObject", "Component");
    }

    void API::RegisterResourceManager(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("IResource", "ResourceManager", {
                { "bool",                   "m_autoRemove",    EvoScript::Protected },
                { "volatile bool",          "m_isDestroy",     EvoScript::Protected },
                { "volatile unsigned long", "m_countUses",     EvoScript::Protected },
                { "const char*",            "m_resource_name", EvoScript::Protected },
                { "std::string",            "m_resource_id",   EvoScript::Protected },
        }, {"string"});
        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Private, generator, IResource, Free, void)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, IsEnabledAutoRemove, bool)
        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Public, generator, IResource, Destroy, bool)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, IsDestroy, bool)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, AddUsePoint, void)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, RemoveUsePoint, void)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, GetCountUses, unsigned int)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, GetResourceName, const char*)
        ESRegisterMethodArg0(Helper::,        EvoScript::Public, generator, IResource, GetResourceID, std::string)
    }

    void API::RegisterMesh(EvoScript::AddressTableGen *generator) {
        generator->AddIncompleteType("Mesh", "Mesh");

        generator->RegisterNewClass("Mesh", "Mesh", {
                { "bool",                  "m_inverse",       EvoScript::Protected },

                { "void*",                 "m_env",           EvoScript::Protected }, // TODO
                { "int",                   "m_pipeline",      EvoScript::Protected },

                { "mutable std::mutex",    "m_mutex",         EvoScript::Protected },

                { "std::string",           "m_geometry_name", EvoScript::Protected },
                { "void*",                 "m_shader",        EvoScript::Protected },
                { "void*",                 "m_render",        EvoScript::Protected },
                { "Material*",             "m_material",      EvoScript::Protected },

                { "volatile bool",         "m_hasErrors",     EvoScript::Protected },
                { "volatile bool",         "m_isCalculated",  EvoScript::Protected },

                { "int32_t",               "m_descriptorSet", EvoScript::Protected },
                { "int32_t",               "m_VBO",           EvoScript::Protected },
                { "int32_t",               "m_IBO",           EvoScript::Protected },
                { "int32_t",               "m_UBO",           EvoScript::Protected },

                { "std::vector<uint32_t>", "m_indices",       EvoScript::Protected },
                { "uint32_t",              "m_countVertices", EvoScript::Protected },
                { "uint32_t",              "m_countIndices",  EvoScript::Protected },
                { "bool",                  "m_useIndices",    EvoScript::Protected },

                { "Vector3",               "m_barycenter",    EvoScript::Protected },
                { "Vector3",               "m_position",      EvoScript::Protected },
                { "Vector3",               "m_rotation",      EvoScript::Protected },
                { "Vector3",               "m_scale",         EvoScript::Protected },
                { "_64byte",               "m_modelMat",      EvoScript::Protected },
        }, {
            "ResourceManager.h", "Component.h", "mutex", "Math/Vector3.h", "Utils.h", "Material.h"
        }, { { "IResource", EvoScript::Public }, { "Component", EvoScript::Public } });

        ESRegisterStaticMethod(Graphics::, EvoScript::Public, generator, Mesh, Load, std::vector<Mesh*>, ESArg1(const std::string& path), ESArg1(path))

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, DrawOnInspector, bool) // Component
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, GetBarycenter, Vector3) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Public, generator, Mesh, OnMove, void, ESArg1(Vector3 v), ESArg1(v)) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Public, generator, Mesh, OnRotate, void, ESArg1(Vector3 v), ESArg1(v)) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Public, generator, Mesh, OnScaled, void, ESArg1(Vector3 v), ESArg1(v)) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Public, generator, Mesh, OnSelected, void, ESArg1(bool v), ESArg1(v)) // Component

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, Destroy, bool) // IResource

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, OnDestroyGameObject, void) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Public, generator, Mesh, OnReady, void, ESArg1(bool v), ESArg1(v)) // Component
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, OnAttachComponent, void) // Component

        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Mesh, WaitCalculate, void)
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Mesh, GetMaterial, Material*)
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Mesh, Copy, Mesh*)
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Mesh, GetGeometryName, std::string)

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Public, generator, Mesh, OnRemoveComponent, void) // Component

        generator->AddIncompleteType("Render", "Mesh");
    }

    void API::RegisterGameObject(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("GameObject", "GameObject", {
                { "SafePtr<GameObject>",              "m_this",   EvoScript::Private },
                { "bool",                             "m_isSelect",   EvoScript::Private },
                { "GameObject*",                      "m_parent",     EvoScript::Private },
                { "std::vector<SafePtr<GameObject>>", "m_children",   EvoScript::Private },
                { "uint32_t",                         "m_countChild", EvoScript::Private },
                { "bool",                             "m_isDestroy",  EvoScript::Private },
                { "std::recursive_mutex",             "m_mutex",      EvoScript::Private },
                { "Scene*",                           "m_scene",      EvoScript::Private },
                { "Transform*",                       "m_transform",  EvoScript::Private },
                { "std::vector<Component*>",          "m_components", EvoScript::Private },
                { "std::string",                      "m_name",       EvoScript::Private },
                { "std::string",                      "m_tag",        EvoScript::Private },
        }, { "Math/Vector3.h", "string", "vector", "mutex", "Component.h", "Transform.h", "Types/SafePointer.h", "ISavable.h" },
        { { "ISavable", EvoScript::Public } });

        ESRegisterMethod(Helper::, EvoScript::Public, generator, GameObject, AddComponent, bool, ESArg1(Component* comp), ESArg1(comp))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, GameObject, AddChild, bool, ESArg1(const SafePtr<GameObject>& child), ESArg1(child))
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, GameObject, GetTransform, Transform*)
        ESRegisterMethod(Helper::, EvoScript::Public, generator, GameObject, GetComponent, Component*, ESArg1(const std::string& name), ESArg1(name))
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, GameObject, GetBarycenter, Vector3)

        using namespace Xml;

        ESRegisterMethodOverrideArg0(Helper::, EvoScript::Public, generator, GameObject, Save, Document)
        ESRegisterMethodOverride(Helper::, EvoScript::Public, generator, GameObject, Load, bool, ESArg1(const Document& xml), ESArg1(xml))

        generator->AddIncompleteType("Scene", "GameObject");
    }

    void API::RegisterCamera(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Camera", "Camera", {
                { "volatile bool",   "m_isCreate",          EvoScript::Private },
                { "volatile bool",   "m_isCalculate",       EvoScript::Private },
                { "volatile bool",   "m_isBuffCalculate",   EvoScript::Private },
                { "volatile bool",   "m_needUpdate",        EvoScript::Private },
                { "volatile float",  "m_yaw",               EvoScript::Private },
                { "volatile float",  "m_pitch",             EvoScript::Private },
                { "volatile float",  "m_roll",              EvoScript::Private },
                { "PostProcessing*", "m_postProcessing",    EvoScript::Private },
                { "void*",           "m_env",               EvoScript::Private },
                { "int",             "m_pipeline",          EvoScript::Private },
                { "void*",           "m_window",            EvoScript::Private },
                { "_64byte",         "m_projection",        EvoScript::Private },
                { "_64byte",         "m_viewTranslateMat",  EvoScript::Private },
                { "_64byte",         "m_viewMat",           EvoScript::Private },
                { "Vector3",         "m_pos",               EvoScript::Private },
                { "_128byte",        "m_ubo",               EvoScript::Private },
                { "bool",            "m_isEnableDirectOut", EvoScript::Private },
                { "bool",            "m_allowUpdateProj",   EvoScript::Private },
                { "float",           "m_far",               EvoScript::Private },
                { "float",           "m_near",              EvoScript::Private },
                { "void*",           "m_canvas",            EvoScript::Private },
                { "Vector2",         "m_cameraSize",        EvoScript::Private },
        }, { "Math/Vector3.h", "Math/Vector2.h", "Utils.h", "Component.h", "PostProcessing.h" }, {
                { "Component", EvoScript::Public }
        });

        ESRegisterStaticMethod(Graphics::, EvoScript::Public, generator, Camera, Allocate, Camera*, ESArg2(uint32_t w, uint32_t h), ESArg2(w, h))

        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Camera, SetDirectOutput, void, ESArg1(bool v), ESArg1(v))
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Camera, GetPostProcessing, PostProcessing*)
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Camera, IsDirectOutput, bool)

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Camera, DrawOnInspector, bool) // Component
        //ESRegisterOverrideMethod(Graphics::, EvoScript::Private, generator, Camera, GetBarycenter, Vector3, () const, "Component")
        ESRegisterMethodOverride(Graphics::, EvoScript::Private, generator, Camera, OnRotate, void, ESArg1(Vector3 v), ESArg1(v)) // Component
        ESRegisterMethodOverride(Graphics::, EvoScript::Private, generator, Camera, OnMove, void, ESArg1(Vector3 v), ESArg1(v)) // Component
        //ESRegisterOverrideMethod(Graphics::, EvoScript::Private, generator, Camera, OnScaled, void, (Vector3), "Component")
        //ESRegisterOverrideMethod(Graphics::, EvoScript::Private, generator, Camera, OnSelected, void, (bool), "Component")
        ESRegisterMethodOverride(Graphics::, EvoScript::Private, generator, Camera, OnReady, void, ESArg1(bool v), ESArg1(v)) // Component
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Camera, OnAttachComponent, void) // Component

        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Camera, OnRemoveComponent, void) // Component
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Camera, OnDestroyGameObject, void) // Component
    }

    void API::RegisterRender(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("MeshCluster", "Render", {
                { "std::map<uint32_t, std::vector<Mesh*>>", "MeshGroups",        EvoScript::Private },
                { "std::map<uint32_t, uint32_t>",           "MeshGroupCounters", EvoScript::Private },
                { "uint32_t",                               "m_total",           EvoScript::Private },
        });

        const std::string renderSkybox = "[" + std::to_string(sizeof(RenderSkybox)) + "]";

        generator->RegisterNewClass("Render", "Render", {
                { "volatile bool",        "m_isCreate",               EvoScript::Private },
                { "volatile bool",        "m_isInit",                 EvoScript::Private },
                { "volatile bool",        "m_isRun",                  EvoScript::Private },
                { "volatile bool",        "m_isClose",                EvoScript::Private },

                { "void*",                "m_env",                    EvoScript::Private },

                { "Window*",              "m_window",                 EvoScript::Private },
                { "Camera*",              "m_currentCamera",          EvoScript::Private },
                { "std::mutex",           "m_mutex",                  EvoScript::Private },

                { "std::vector<Mesh*>",   "m_newMeshes",              EvoScript::Private },
                { "std::vector<Mesh*>",   "m_removeMeshes",           EvoScript::Private },
                { "std::vector<void*>",   "m_textureToFree",          EvoScript::Private },
                { "std::vector<Skybox*>", "m_skyboxesToFreeVidMem",   EvoScript::Private },

                { "MeshCluster",           "m_geometry",              EvoScript::Private },
                { "MeshCluster",           "m_transparentGeometry",   EvoScript::Private },
                { "MeshCluster",           "m_wireframeGeometry",     EvoScript::Private },

                { "char",                  "m_skybox" + renderSkybox, EvoScript::Private },

                { "std::vector<void*>",    "m_shaders",               EvoScript::Private },

                { "void*",                 "m_grid",                  EvoScript::Private },
                { "void*",                 "m_colorBuffer",           EvoScript::Private },

                { "bool",                  "m_gridEnabled",           EvoScript::Private },
                { "bool",                  "m_skyboxEnabled",         EvoScript::Private },
                { "bool",                  "m_wireFrame",             EvoScript::Private },

                { "int32_t",               "m_pipeLine",              EvoScript::Private },
        }, { "vector", "mutex", "Utils.h", "stdint.h", "map", "Skybox.h", "Texture.h" });

        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, UpdateUBOs, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawGeometry, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawDebugWireframe, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawSkybox, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawGrid, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawSingleColors, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawTransparentGeometry, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, Render, DrawSettingsPanel, bool)

        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Render, SetSkybox, void, ESArg1(Skybox* skybox), ESArg1(skybox))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Render, RegisterMesh, void, ESArg1(Mesh* mesh), ESArg1(mesh))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Render, RegisterTexture, void, ESArg1(Texture* texture), ESArg1(texture))

        generator->AddIncompleteType("Window", "Render");
        generator->AddIncompleteType("Camera", "Render");
        generator->AddIncompleteType("Mesh", "Render");
    }

    void API::RegisterWindow(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Window", "Window", {
                { "volatile bool",        "m_isCreate",              EvoScript::Private },
                { "volatile bool",        "m_isInit",                EvoScript::Private },
                { "volatile bool",        "m_isRun",                 EvoScript::Private },
                { "volatile bool",        "m_isClose",               EvoScript::Private },
                { "volatile bool",        "m_hasErrors",             EvoScript::Private },
                { "volatile bool",        "m_isEnvInit",             EvoScript::Private },
                { "volatile bool",        "m_isWindowClose",         EvoScript::Private },
                { "volatile bool",        "m_isWindowFocus",         EvoScript::Private },
                { "volatile bool",        "m_isNeedResize",          EvoScript::Private },
                { "volatile bool",        "m_isNeedMove",            EvoScript::Private },
                { "std::thread",          "m_thread",                EvoScript::Private },
                { "void*",                "m_time",                  EvoScript::Private },
                { "void*",                "m_env",                   EvoScript::Private },
                { "const char*",          "m_win_name",              EvoScript::Private },
                { "const char*",          "m_icoPath",               EvoScript::Private },
                { "uint8_t",              "m_smoothSamples",         EvoScript::Private },
                { "void*",                "m_render",                EvoScript::Private },
                { "std::mutex",           "m_camerasMutex",          EvoScript::Private },
                { "std::vector<Camera*>", "m_newCameras",            EvoScript::Private },
                { "uint32_t",             "m_countNewCameras",       EvoScript::Private },
                { "std::vector<Camera*>", "m_camerasToDestroy",      EvoScript::Private },
                { "uint32_t",             "m_countCamerasToDestroy", EvoScript::Private },
                { "std::vector<Camera*>", "m_cameras",               EvoScript::Private },
                { "uint32_t",             "m_countCameras",          EvoScript::Private },
                { "bool",                 "m_GUIEnabled",            EvoScript::Private },
                { "void*",                "m_aimedWindowTarget",     EvoScript::Private },
                { "Camera*",              "m_aimedCameraTarget",     EvoScript::Private },
                { "Mesh*",                "m_aimedMesh",             EvoScript::Private },
                { "bool",                 "m_requireGetAimed",       EvoScript::Private },
                { "void*",                "m_canvas",                EvoScript::Private },
                { "bool",                 "m_vsync",                 EvoScript::Private },
                { "bool",                 "m_fullScreen",            EvoScript::Private },
                { "bool",                 "m_resizable",             EvoScript::Private },
                { "Vector2",              "m_windowPos",             EvoScript::Private },
                { "Vector2",              "m_newWindowPos",          EvoScript::Private },
                { "Vector2",              "m_newWindowSize",         EvoScript::Private },
        }, { "thread", "vector", "mutex", "stdint.h", "Math/Vector2.h" });

        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Window, AddCamera, void, ESArg1(Camera* camera), ESArg1(camera))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Window, Resize, void, ESArg2(uint32_t w, uint32_t h), ESArg2(w, h))
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Window, CentralizeWindow, void)
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Window, SetGUIEnabled, void, ESArg1(bool v), ESArg1(v))
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Window, GetWindowSize, Vector2)
        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, Window, IsGUIEnabled, bool)

        generator->AddIncompleteType("Camera", "Window");
        generator->AddIncompleteType("Mesh", "Window");
    }

    void API::RegisterTransform(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Transform", "Transform", {
                { "Vector3",     "m_localPosition",  EvoScript::Private },
                { "Vector3",     "m_localRotation",  EvoScript::Private },
                { "Vector3",     "m_localScale",     EvoScript::Private },

                { "Vector3",     "m_globalPosition", EvoScript::Private },
                { "Vector3",     "m_globalRotation", EvoScript::Private },
                { "Vector3",     "m_globalScale",    EvoScript::Private },

                { "GameObject*", "m_gameObject",     EvoScript::Private },
                { "Transform*",  "m_parent",         EvoScript::Private },
        }, { "Math/Vector3.h" });

        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Transform, Forward, Vector3)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Transform, Right, Vector3)
        ESRegisterMethodArg0(Helper::, EvoScript::Public, generator, Transform, Up, Vector3)
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Transform, Translate, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Transform, Rotate, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Transform, Scaling, void, ESArg1(Vector3 v), ESArg1(v))
        ESRegisterMethod(Helper::, EvoScript::Public, generator, Transform, RotateAround, void, ESArg4(Vector3 point, Vector3 axis, Unit angle, bool local), ESArg4(point, axis, angle, local))

        generator->AddIncompleteType("GameObject", "Transform");
    }

    void API::RegisterInput(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Input", "Input", {}, { "Math/Vector2.h" });
        ESRegisterStaticMethodArg0(Helper::, EvoScript::Public, generator, Input, GetMouseWheel, int)
        ESRegisterStaticMethodArg0(Helper::, EvoScript::Public, generator, Input, GetMouseDrag, Vector2)
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Input, GetKey, bool, ESArg1(KeyCode key), ESArg1(key))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Input, GetKeyDown, bool, ESArg1(KeyCode key), ESArg1(key))
        ESRegisterStaticMethod(Helper::, EvoScript::Public, generator, Input, GetKeyUp, bool, ESArg1(KeyCode key), ESArg1(key))

        generator->RegisterEnum("KeyCode", "Input", true, {
                { "MouseLeft",  1 },
                { "MouseRight", 2 },
                { "MouseMiddle", 4 },
                { "BackSpace", 8 },
                { "Tab", 9 },
                { "Enter", 13 },
                { "LShift", 16 },
                { "Ctrl", 17 },
                { "Alt", 18 },
                { "Esc", 27 },
                { "Space", 32 },
                { "LeftArrow", 37 },
                { "UpArrow", 38 },
                { "RightArrow", 39 },
                { "DownArrow", 40 },
                { "Del", 46 },
                { "A", 65 },
                { "B", 66 },
                { "C", 67 },
                { "D", 68 },
                { "E", 69 },
                { "F", 70 },
                { "G", 71 },
                { "H", 72 },
                { "I", 73 },
                { "J", 74 },
                { "K", 75 },
                { "L", 76 },
                { "M", 77 },
                { "N", 78 },
                { "O", 79 },
                { "P", 80 },
                { "Q", 81 },
                { "S", 83 },
                { "R", 82 },
                { "T", 84 },
                { "U", 85 },
                { "V", 86 },
                { "W", 87 },
                { "X", 88 },
                { "Y", 89 },
                { "Z", 90 },
                { "F1", 112 },
                { "F2", 113 },
                { "F3", 114 },
                { "F4", 115 },
                { "F5", 116 },
                { "F6", 117 },
                { "F7", 118 },
                { "F8", 119 },
                { "F9", 120 },
                { "F10", 121 },
                { "F11", 122 },
                { "F12", 123 },
        });
    }

    void API::RegisterSkybox(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Skybox", "Skybox", {
                { "void*",                   "m_env",            EvoScript::Private },
                { "void*",                   "m_render",         EvoScript::Private },
                { "void*",                   "m_shader",         EvoScript::Private },
                { "int32_t",                 "m_VAO",            EvoScript::Private },
                { "int32_t",                 "m_VBO",            EvoScript::Private },
                { "int32_t",                 "m_IBO",            EvoScript::Private },
                //{ "int32_t",                 "m_UBO",            EvoScript::Private },
                { "int32_t",                 "m_descriptorSet",  EvoScript::Private },
                { "int32_t",                 "m_cubeMap",        EvoScript::Private },
                { "uint32_t",                "m_width",          EvoScript::Private },
                { "uint32_t",                "m_height",         EvoScript::Private },
                { "std::array<uint8_t*, 6>", "m_data",           EvoScript::Private },
                { "bool",                    "m_isCalculated",   EvoScript::Private },
                { "bool",                    "m_hasErrors",      EvoScript::Private },
                { "volatile bool",           "m_isVideoMemFree", EvoScript::Private },
                { "std::string",             "m_name",           EvoScript::Private },
        }, { "stdint.h", "vector", "string", "array" });

        ESRegisterStaticMethod(Graphics::, EvoScript::Public, generator, Skybox, Load, Skybox*, ESArg1(const std::string& path), ESArg1(path))
    }

    void API::RegisterTexture(EvoScript::AddressTableGen *generator) {
        generator->RegisterEnum("TextureCompression", "Texture", true, {
                { "None", 0 },
                { "BC1",  1 },
                { "BC2",  2 },
                { "BC3",  3 },
                { "BC4",  4 },
                { "BC5",  5 },
                { "BC6",  6 },
                { "BC7",  7 },
        });

        generator->RegisterEnum("TextureFormat", "Texture", true, {
                { "Unknown",          0 },
                { "RGBA8_UNORM",  10000 },
                { "RGBA16_UNORM", 10001 },
                { "RGBA8_SRGB",   20000 },
        });

        generator->RegisterEnum("TextureType", "Texture", true, {
                { "Unknown",    0 },
                { "Diffuse",    1 },
                { "Normal",     2 },
                { "Specular",   3 },
                { "Roughness",  4 },
                { "Glossiness", 5 },
        });

        generator->RegisterEnum("TextureFilter", "Texture", true, {
                { "Unknown",    0 },
                { "NEAREST",    1 },
                { "LINEAR",     2 },
        });

        generator->RegisterNewClass("Texture", "Texture", {
                { "int32_t",            "m_ID",          EvoScript::Private },
                { "uint32_t",           "m_width",       EvoScript::Private },
                { "uint32_t",           "m_height",      EvoScript::Private },
                { "TextureFormat",      "m_format",      EvoScript::Private },
                { "TextureCompression", "m_compression", EvoScript::Private },
                { "uint8_t",            "m_mipLevels",   EvoScript::Private },
                { "bool",               "m_alpha",       EvoScript::Private },
                { "uint8_t*",           "m_data",        EvoScript::Private },
                { "volatile bool",      "m_isCalculate", EvoScript::Private },
                { "bool",               "m_hasErrors",   EvoScript::Private },
                { "void*",              "m_render",      EvoScript::Private },
                { "std::mutex",         "m_mutex",       EvoScript::Private },
                { "TextureType",        "m_type",        EvoScript::Private },
                { "TextureFilter",      "m_filter",      EvoScript::Private },
        }, { "mutex", "stdint.h", "ResourceManager.h", "string" }, { { "IResource", EvoScript::Public } });
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Texture, Destroy, bool) // IResource
        ESRegisterMethodOverrideArg0(Graphics::, EvoScript::Private, generator, Texture, Free, void) // IResource
        ESRegisterStaticMethod(Graphics::, EvoScript::Public, generator, Texture, Load, Texture*,
                ESArg7(const std::string& path, TextureFormat format, bool autoRemove, TextureType type, TextureFilter filter, TextureCompression compress, uint8_t mip),
                ESArg7(path, format, autoRemove, type, filter, compress, mip))
    }

    void API::RegisterMaterial(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Material", "Material", {
                { "bool",          "m_transparent",    EvoScript::Private },
                { "volatile bool", "m_bloom",          EvoScript::Private },
                { "_16byte",       "m_color",          EvoScript::Private },
                { "Mesh*",         "m_mesh",           EvoScript::Private },
                { "volatile bool", "m_texturesIsFree", EvoScript::Private },
                { "Texture*",      "m_diffuse",        EvoScript::Private },
                { "Texture*",      "m_normal",         EvoScript::Private },
                { "Texture*",      "m_specular",       EvoScript::Private },
                { "Texture*",      "m_glossiness",     EvoScript::Private },
        }, { "Utils.h", "Texture.h" });

        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Material, SetDiffuse, void, ESArg1(Texture* texture), ESArg1(texture))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Material, SetNormal, void, ESArg1(Texture* texture), ESArg1(texture))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Material, SetSpecular, void, ESArg1(Texture* texture), ESArg1(texture))
        ESRegisterMethod(Graphics::, EvoScript::Public, generator, Material, SetGlossiness, void, ESArg1(Texture* texture), ESArg1(texture))

        generator->AddIncompleteType("Mesh", "Material");
    }

    void API::RegisterGUISystem(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("GUISystem", "GUISystem", { // TODO
                { "void*",                      "m_env",         EvoScript::Private },
                { "int32_t",                    "m_pipeLine",    EvoScript::Private },
                { "std::map<uint32_t, void*>", "m_descriptors", EvoScript::Private },
        }, { "cstdint", "Math/Vector2.h", "map", "Scene.h", "Camera.h", "GameObject.h" });
        ESRegisterStaticMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, Get, GUISystem*)
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, BeginMenuBar, bool)
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, EndMenuBar, void)
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, BeginDockSpace, void)
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, EndDockSpace, void)
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, BeginWindow, bool, ESArg1(const char* name), ESArg1(name))
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, EndWindow, void)
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, BeginChildWindow, bool, ESArg1(const char* name), ESArg1(name))
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, EndChildWindow, void)
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, DrawTexture, void, ESArg4(Vector2 win, Vector2 img, uint32_t id, bool center), ESArg4(win, img, id, center))
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, DrawHierarchy, void, ESArg1(SafePtr<Scene> scene), ESArg1(scene))
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, DrawInspector, void, ESArg1(SafePtr<Scene> scene), ESArg1(scene))
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, GetWindowSize, Vector2)
        ESRegisterMethodArg0(GUI::, EvoScript::Public, generator, GUISystem, DrawGuizmoTools, void)
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, SetGuizmoTool, void, ESArg1(uint8_t id), ESArg1(id))
        ESRegisterMethod(GUI::, EvoScript::Public, generator, GUISystem, DrawGuizmo, void, ESArg2(Camera* camera, SafePtr<GameObject> gm), ESArg2(camera, gm))
    }

    void API::RegisterPostProcessing(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("PostProcessing", "PostProcessing", {
                { "float",                "m_gamma",                 EvoScript::Protected },
                { "float",                "m_exposure",              EvoScript::Protected },
                { "float",                "m_saturation",            EvoScript::Protected },
                { "Vector3",              "m_colorCorrection",       EvoScript::Protected },
                { "Vector3",              "m_bloomColor",            EvoScript::Protected },
                { "float",                "m_bloomIntensity",        EvoScript::Protected },
                { "volatile uint8_t",     "m_bloomAmount",           EvoScript::Protected },

                { "volatile bool",        "m_bloom",                 EvoScript::Protected },
                { "bool",                 "m_bloomClear",            EvoScript::Protected },

                { "bool",                 "m_horizontal",            EvoScript::Protected },
                { "bool",                 "m_firstIteration",        EvoScript::Protected },

                { "int32_t",              "m_finalDepth",            EvoScript::Protected },
                { "int32_t",              "m_finalFBO",              EvoScript::Protected },
                { "int32_t",              "m_finalColorBuffer",      EvoScript::Protected },

                { "std::vector<int32_t>", "m_colors",                EvoScript::Protected },
                { "int32_t",              "m_depth",                 EvoScript::Protected },
                { "int32_t",              "m_frameBuffer",           EvoScript::Protected },

                { "int32_t",               "m_descriptorSet",        EvoScript::Protected },
                { "int32_t",               "m_ubo",                  EvoScript::Protected },

                { "void*",                 "m_env",                  EvoScript::Protected },
                { "Shader*",               "m_postProcessingShader", EvoScript::Protected },
                { "Shader*",               "m_blurShader",           EvoScript::Protected },

                { "Camera*",               "m_camera",               EvoScript::Protected },
                { "Render*",               "m_render",               EvoScript::Protected },
                { "bool",                  "m_isInit",               EvoScript::Protected },
        }, { "cstdint", "Math/Vector3.h", "vector" });

        ESRegisterMethodVirtual(Graphics::, EvoScript::Private, generator, PostProcessing, Init, bool, ESArg1(Render* render), ESArg1(render))
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, Destroy, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, Free, bool)
        ESRegisterMethodVirtual(Graphics::, EvoScript::Private, generator, PostProcessing, OnResize, bool, ESArg2(uint32_t w, uint32_t h), ESArg2(w, h))
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, BeginSkybox, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, EndSkybox, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, BeginGeometry, bool)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, EndGeometry, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, Complete, void)
        ESRegisterMethodVirtualArg0(Graphics::, EvoScript::Private, generator, PostProcessing, Draw, void)

        ESRegisterMethodArg0(Graphics::, EvoScript::Public, generator, PostProcessing, GetFinally, uint32_t)

        generator->AddIncompleteType("Shader", "PostProcessing");
        generator->AddIncompleteType("Camera", "PostProcessing");
        generator->AddIncompleteType("Render", "PostProcessing");
    }

    void API::RegisterISavable(EvoScript::AddressTableGen *generator) {
        generator->RegisterNewClass("Document", "ISavable", {
                { "char", "b[" + std::to_string(sizeof(Xml::Document)) + "]", EvoScript::Public }
        });

        generator->RegisterNewClass("ISavable", "ISavable", {
                { "char", "b[" + std::to_string(sizeof(ISavable)) + "]", EvoScript::Public }
        });

        using namespace Xml;

        ESRegisterMethodVirtualArg0(Helper::, EvoScript::Protected, generator, ISavable, Save, Document)
        ESRegisterMethodVirtual(Helper::, EvoScript::Protected, generator, ISavable, Load, bool, ESArg1(const Document& xml), ESArg1(xml))
    }
}

