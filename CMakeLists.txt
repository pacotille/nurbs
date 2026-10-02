cmake_minimum_required(VERSION 3.16)
project(CubeDemo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
# Find packages
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)



pkg_check_modules(CHIBI REQUIRED chibi-scheme)

# ImGui (vendored or via FetchContent)
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.4
)

FetchContent_MakeAvailable(imgui)

# ImGui sources
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/intersection.cpp
    ${IMGUI_SOURCES}
)


target_include_directories(${PROJECT_NAME} PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${SDL2_INCLUDE_DIRS}
    ${INSTALL_DIR}/include
    ${CHIBI_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    OpenGL::GL
    ${CMAKE_DL_LIBS}
    ${CHIBI_LIBRARIES} 
)

#file(COPY scripts DESTINATION ${CMAKE_BINARY_DIR})

message(STATUS "Inc ${CHIBI_INCLUDE_DIRS}")
message(STATUS "Lib ${CHIBI_LIBRARIES}")
message(STATUS "gg chibi-scheme")
message(STATUS "dd ${chibi-scheme_SOURCE_DIR}/include")

# Platform-specific GL loader
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE GL)
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework OpenGL")
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)
endif()
