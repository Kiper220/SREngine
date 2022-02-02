//
// Created by Nikita on 29.12.2020.
//

#include <macros.h>
#include <Debug.h>

#include <Engine.h>

#include <ResourceManager/ResourceManager.h>
#include <Environment/OpenGL.h>
#include <Environment/Vulkan.h>
#include <EntityComponentSystem/Transform.h>

#include <Types/Rigidbody.h>
#include <Types/Geometry/Mesh3D.h>
#include <Animations/Bone.h>
#include <World/World.h>
#include <Input/InputSystem.h>
#include <Memory/MeshAllocator.h>
#include <World/VisualChunk.h>
#include <World/VisualRegion.h>
#include <Utils/CmdOptions.h>
#include <Utils/Features.h>
#include <GUI/NodeManager.h>

using namespace Framework;

using namespace Framework::Core;
using namespace Framework::Core::World;

using namespace Framework::Helper;
using namespace Framework::Helper::Math;
using namespace Framework::Helper::Types;
using namespace Framework::Helper::World;

using namespace Framework::Graphics;
using namespace Framework::Graphics::Types;
using namespace Framework::Graphics::Animations;

using namespace Framework::Physics;
using namespace Framework::Physics::Types;

/*
        +---------------+       +----+          +----+       +---------------+            +----------------+
        |               |       \     \        /     /       |               |           /                 |
        |               |        \     \      /     /        |               |          /                  |
        |      +--------+         \     \    /     /         |      +--------+         /         +---------+
        |      |                   \     \  /     /          |      |                 /        /
        |      +--------+           \     \/     /           |      +--------+       |        /
        |               |            \          /            |               |       |       |
        |               |            /          \            |               |       |       |
        |      +--------+           /     /\     \           |      +--------+       |        \
        |      |                   /     /  \     \          |      |                 \        \
        |      +--------+         /     /    \     \         |      +--------+         \        +---------+
        |               |        /     /      \     \        |               |          \                 |
        |               |       /     /        \     \       |               |           \                |
        +---------------+       +----+          +-----+      +---------------+            +---------------+
 */

int main(int argc, char **argv) {
    if constexpr (sizeof(size_t) != 8) {
        std::cerr << "The engine only supports 64-bit systems!\n";
        return -1;
    }

    std::string exe = FileSystem::GetPathToExe();
    Debug::Init(exe, true, Debug::Theme::Dark);
    Debug::SetLevel(Debug::Level::Low);

    if (auto folder = GetCmdOption(argv, argv + argc, "-resources"); folder.empty())
        ResourceManager::Instance().Init(exe + "/../../Resources");
    else
        ResourceManager::Instance().Init(folder);

    Features::Instance().Reload(ResourceManager::Instance().GetResPath().Concat("/Configs/Features.xml"));

    if (Features::Instance().Enabled("CrashHandler")) {
#ifdef SR_WIN32
        ShellExecute(nullptr, "open", (ResourceManager::Instance().GetResPath().Concat(
                "/Utilities/EngineCrashHandler.exe").CStr()),
                     ("--log log.txt --target " + FileSystem::GetExecutableFileName() + " --out " + exe + "\\").c_str(),
                     nullptr, SW_SHOWDEFAULT
        );
#endif
    }

    // Register all resource types
    {
        ResourceManager::Instance().RegisterType<Mesh>();
        ResourceManager::Instance().RegisterType<Texture>();
        ResourceManager::Instance().RegisterType<Material>();
    }

    // Register all components
    {
        //Component::RegisterComponent("SkinnedMesh", []() -> Component* { return new SkinnedMesh();  });
        Component::RegisterComponent("Mesh3D",        []() -> Mesh3D*    { return Memory::MeshAllocator::Allocate<Mesh3D>(); });
        Component::RegisterComponent("Rigidbody",     []() -> Rigidbody* { return new Rigidbody();                           });
        Component::RegisterComponent("Camera",        []() -> Camera*    { return Camera::Allocate();                        });
        Component::RegisterComponent("Bone",          []() -> Bone*      { return new Bone();                                });

        Component::RegisterEvents("Bone", [](Component* bone){
            dynamic_cast<Bone*>(bone)->SetRender(Engine::Instance().GetRender());
        });

        if (Helper::Features::Instance().Enabled("DebugChunks", false))
            Chunk::SetAllocator([](SRChunkAllocArgs) -> Chunk * { return new VisualChunk(SRChunkAllocVArgs); });

        if (Helper::Features::Instance().Enabled("DebugRegions", false))
            Region::SetAllocator([](SRRegionAllocArgs) -> Region* { return new VisualRegion(SRRegionAllocVArgs); });

        Scene::SetAllocator([](const std::string& name) -> Scene* { return new Core::World::World(name); });
    }

    if (const auto env = Helper::FileSystem::ReadAllText(ResourceManager::Instance().GetResPath().Concat("/Configs/Environment.config")); env == "OpenGL")
        Environment::Set(new OpenGL());
    else if (env == "Vulkan")
        Environment::Set(new Vulkan());
    else if (env.empty()) {
        Helper::Debug::Error("System error: file \"Resources/Configs/Environment.config\" does not exist!\n\t"
                             "Please, create it and write the name of the environment there!");
        ResourceManager::Instance().Stop();
        Debug::Stop();
        return -1500;
    } else {
        Helper::Debug::Error("System error: unknown environment! \"" + env + "\" does not support!");
        ResourceManager::Instance().Stop();
        Debug::Stop();
        return -2000;
    }

    Render* render = Render::Allocate();
    if (!render) {
        Helper::Debug::Error("FATAL: render is not support this pipeline!");
        ResourceManager::Instance().Stop();
        Debug::Stop();
        return -1000;
    }

    auto window = new Window(
            "SpaRcle Engine",
            "Engine/icon.ico",
            WindowFormat::_1366_768,
            render,
            false, // vsync
            false, // fullscreen
            true,  // resizable
            true,  // header enabled
            2
    );

    auto physics = new PhysEngine();

    auto&& engine = Engine::Instance();

    if(engine.Create(window, physics)) {
      if (engine.Init(Engine::MainScriptType::Engine)) {
          if (engine.Run()){

          }
          else
              Debug::Error("Failed to running game engine!");
      } else
          Debug::Error("Failed to initializing game engine!");
    } else
        Debug::Error("Failed to creating game engine!");

    if (engine.IsRun()) {
        Debug::System("All systems successfully run!");

        engine.Await(); // await close engine
    }

    engine.Close();

    Framework::Helper::EntityManager::Destroy();
    Framework::Engine::Destroy();
    Framework::Graphics::GUI::NodeManager::Destroy();

    Debug::System("All systems successfully closed!");

    ResourceManager::Instance().Stop();

    return Debug::Stop();
}