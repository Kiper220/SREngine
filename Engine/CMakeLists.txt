cmake_minimum_required(VERSION 3.16)
project(SpaRcleEngine)

add_subdirectory(Core)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

add_executable(SREngine
        main.cpp
        ${APP_ICON_RESOURCE_WINDOWS}
)

add_custom_command(TARGET SREngine
        POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -DTARGET_NAME=Engine
        -DTARGET_PATH=${CMAKE_BINARY_DIR}
        -P ${CMAKE_SOURCE_DIR}/Engine/post_build.cmake
        COMMENT "Running SpaRcle engine post-build script..."
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

target_link_libraries(SREngine Core)

