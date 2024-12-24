cmake_minimum_required(VERSION 3.8)

project(Ibex)

cmake_policy(SET CMP0072 NEW)
find_package(OpenGL REQUIRED)

add_executable(Ibex
    "src/GLAD/glad.c"

    "src/ResourceManager/MaterialLibrary.cpp"
    "src/ResourceManager/ResourceManager.cpp"
    "src/ResourceManager/TextureData.cpp"
    "src/ResourceManager/ShaderData.cpp"
    "src/ResourceManager/MeshData.cpp"

    "src/Graphics/ShaderObject.cpp"
    "src/Graphics/TextureObject.cpp"
    "src/Graphics/MeshObject.cpp"

    "src/Engine/Transform.cpp"
    "src/Engine/Node.cpp"
    "src/Engine/InputHandler.cpp"
    "src/Engine/Camera.cpp"

    "src/App.cpp"
)

if (WIN32)
target_include_directories(Ibex PUBLIC
    "src/"
    "src/vendor/"
    "${CMAKE_SOURCE_DIR}/Dependencies/GLFW-static/include"
)
add_library(glfw STATIC IMPORTED)
set_property(TARGET glfw PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Dependencies/GLFW-static/lib-mingw32/libglfw3.a")
else()
find_package(nlohmann_json 3.11.3 REQUIRED)
target_include_directories(Ibex PUBLIC
    "src/"
    "src/vendor/"
)
endif()

set(DEPENDENCY_TARGET ResourceStuff)
add_custom_target(
    ResourceStuff
    COMMENT "Copying Resources"
    COMMAND cp -r ${CMAKE_SOURCE_DIR}/res ${PROJECT_BINARY_DIR}
)
add_dependencies(Ibex ResourceStuff)

if (WIN32)
target_link_libraries(Ibex
    opengl32
    glfw
)
else()
target_link_libraries(Ibex
    OpenGL
    glfw
)
endif()
