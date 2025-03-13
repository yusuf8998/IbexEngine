cmake_minimum_required(VERSION 3.8)

project(Ibex)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

add_executable(Ibex
    "src/KHR/khrplatform.h"
    "src/GLAD/glad.c"

    "src/ResourceManager/MaterialLibrary.cpp"
    "src/ResourceManager/ShaderProgram.cpp"
    "src/ResourceManager/TextureData.cpp"
    "src/ResourceManager/ShaderData.cpp"
    "src/ResourceManager/MeshData.cpp"
    "src/ResourceManager/ResourceManager.cpp"

    "src/ResourceManager/AssetPack/AssetPack.cpp"

    "src/Graphics/GL.h"
    "src/Graphics/ShaderObject.cpp"
    "src/Graphics/TextureObject.cpp"
    "src/Graphics/TextureArrayObject.cpp"
    "src/Graphics/CubemapObject.cpp"
    "src/Graphics/RenderObject.cpp"
    "src/Graphics/ParticleObject.cpp"
    "src/Graphics/InputManager/InputManager.cpp"
    "src/Graphics/LightCaster.cpp"
    "src/Graphics/FramebufferObject.cpp"
    "src/Graphics/ShadowMap.cpp"
    "src/Graphics/UniformBufferObject.cpp"
    "src/Graphics/Renderer.cpp"

    "src/Engine/Transform.cpp"
    "src/Engine/SceneGraph.h"
    "src/Engine/SceneGraph.cpp"
    "src/Engine/SkyboxNode.cpp"
    "src/Engine/BillboardNode.cpp"
    #"src/Engine/InputHandler.cpp"
    "src/Engine/Camera.cpp"
    "src/Engine/LightNode.cpp"

    "src/Engine/FSM/PlayerMachine.cpp"

    "src/Engine/Scene.cpp"

    #ImGui
    "src/vendor/imgui/imgui.cpp" "src/vendor/imgui/imgui_draw.cpp" "src/vendor/imgui/imgui_impl_glfw.cpp" "src/vendor/imgui/imgui_impl_opengl3.cpp"
    "src/vendor/imgui/imgui_tables.cpp" "src/vendor/imgui/imgui_widgets.cpp" "src/vendor/imgui/imgui_stdlib.cpp"

    "src/App.cpp"
)

if (WIN32)
target_include_directories(Ibex PUBLIC
    "src/"
    "src/vendor/"
    "${CMAKE_SOURCE_DIR}/Dependencies/GLFW-static/include"
    "${CMAKE_SOURCE_DIR}/Dependencies/zlib/include"
)
add_library(zlib STATIC IMPORTED)
set_property(TARGET zlib PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Dependencies/zlib/libzlib.dll.a")
add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Dependencies/GLFW-static/lib-mingw32/libglfw3.a")
else()
target_include_directories(Ibex PUBLIC
    "src/"
    "src/vendor/"
)
find_package(nlohmann_json 3.10.5 REQUIRED)
add_library(zlib STATIC IMPORTED)
set_property(TARGET zlib PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Dependencies/zlib/libz.so.1.3.1")
endif()

target_compile_definitions(Ibex PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

set(DEPENDENCY_TARGET ResourceStuff)
add_custom_target(
    ResourceStuff
    COMMENT "Copying Resources"
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/res ${PROJECT_BINARY_DIR}
)
add_dependencies(Ibex ResourceStuff)

set(DEPENDENCY_TARGET DependencyStuff)
add_custom_target(
    DependencyStuff
    COMMENT "Copying Dependencies"
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/Dependencies/zlib/libzlib.dll ${PROJECT_BINARY_DIR}
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/Dependencies/zlib/libz.so.1.3.1 ${PROJECT_BINARY_DIR}
)
add_dependencies(Ibex DependencyStuff)

if (WIN32)
target_link_libraries(Ibex
    opengl32
    glfw
    zlib
)
else()
target_link_libraries(Ibex
    OpenGL
    glfw
    zlib
)
endif()
