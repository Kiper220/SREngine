//
// Created by Monika on 22.11.2022.
//

#include <Physics/PhysicsWorld.h>

namespace SR_PHYSICS_NS {
    PhysicsWorld::PhysicsWorld(LibraryPtr pLibrary, Space space)
        : Super()
        , m_library(pLibrary)
        , m_space(space)
    {
        SRAssert(space != Space::Unknown);
    }
}
