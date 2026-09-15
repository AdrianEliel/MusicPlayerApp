cmake_minimum_required(VERSION 4.3)
project(AdrianMusicPlayer VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 3.0.0
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)
FetchContent_MakeAvailable(SFML)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/imgui")
set(IMGUI_BACKEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/imgui/backends")
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
)

set(IMGUI_SFML_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/thirdparty/imgui-sfml-3.0")
set(IMGUI_SFML_SOURCE
        ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

set(PROJECT_SOURCES
        src/main.cpp
)

add_executable(main ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${IMGUI_SFML_SOURCE})

target_compile_features(main PRIVATE cxx_std_17)

target_include_directories(main PRIVATE ${IMGUI_DIR} ${IMGUI_BACKEND_DIR})
target_include_directories(main PRIVATE ${IMGUI_SFML_DIR})

target_link_libraries(main PRIVATE glfw)
target_link_libraries(main PRIVATE SFML::Graphics)
target_link_libraries(main PRIVATE SFML::Audio)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)
