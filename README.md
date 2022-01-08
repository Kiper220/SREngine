# SpaRcle Engine

## Supported compilers
- [x] MSVC
- [x] MinGW
- [ ] Cygwin
- [ ] Clang
- [ ] GCC

## Supported platforms
- [x] Windows 10
- [ ] Ubuntu
- [ ] Arch
- [ ] Redhat
- [ ] Android

## Used libraries
  * glm
  * OpenGL / GLFW / GLEW / glad
  * Bullet3
  * ImGUI
  * tinyobjloader
  * stbi
  * json
  * EvoScript
  * EvoVulkan

## Exists systems
  * Types
      * Thread - обертка над стандартным классом, с некоторыми дополнениями
      * SafePtr - универсальный умный указатель, обеспечивает сохранность данных в многопоточной среде
      * Singleton
  * Math
      * Quaternion
      * Vector3
      * Vector2
      * Matrix4x4
  * Graphics
      * Mesh-cluster
      * GUI 
      * OBJ-loader
      * FBX-loader
      * Texturing
        * Compressing BC1-BC7 (Only Vulkan)
      * Shaders
      * Materials
      * PostProcessing
        * Saturation (Only OpenGL)
        * Bloom (Slow) (Only OpenGL)
        * Color correction (Base) (Only OpenGL)
  * Physics (Не начато)
      * Rigidbody (Base component)
  * Scripting (Полностью сделано через EvoScript)
  * Audio (Не начато)
  * World
      * Scene 
      * GameObject (Используется частично паттерн ECS)
         * Transform
         * Hierarchy-transformation (80%)
         * Component
      * Chunk-System - чанковая система, которая разбивает мир на чанки и регионы, позволяя удаляться на огромные расстояния от центра мира без каких либо артефактов, производя для этого "сдвиги"
  * Input system (Только стандартый ввод Windows)
  * Memory management
      * ResourceManager
      * IResource - interface for control resource units
  * Debug and logging
 
## Editor
  * Grid (Only OpenGL)
  * Manipulation tools
  * Inspector 
  * Hierarchy
  * Asset explorer
  * World edit
