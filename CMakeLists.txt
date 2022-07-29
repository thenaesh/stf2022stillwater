cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")


set(LIBS_ROOT "~/Libraries")

set(GLM_INCLUDE_DIR "${LIBS_ROOT}/glm-0.9.9.8")

set(GLFW_ROOT "${LIBS_ROOT}/glfw-3.3.7")
set(GLFW_INCLUDE_DIR "${GLFW_ROOT}/include")
set(GLFW_LIBS_DIR "${GLFW_ROOT}/build/src")

set(VULKAN_ROOT "${LIBS_ROOT}/vulkan-1.3.216.0/x86_64")
set(VULKAN_INCLUDE_DIR "${VULKAN_ROOT}/include")
set(VULKAN_LIBS_DIR "${VULKAN_ROOT}/lib")

set(CMAKE_PREFIX_PATH "${GLFW_LIBS_DIR};${VULKAN_LIBS_DIR}")


project(StillWaterVulkan)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/window.cpp
    src/vulkanstate.cpp
    src/shader.cpp
    src/pipeline.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE "include"
    PRIVATE "${GLM_INCLUDE_DIR}"
    PRIVATE "${GLFW_INCLUDE_DIR}"
    PRIVATE "${VULKAN_INCLUDE_DIR}"
)

find_library(GLFW_LIB glfw)
find_library(VULKAN_LIB vulkan)
find_library(DL_LIB dl)
find_library(PTHREAD_LIB pthread)
find_library(X11_LIB X11)
find_library(XXF86VM_LIB Xxf86vm)
find_library(XRANDR_LIB Xrandr)
find_library(XI_LIB Xi)

target_link_libraries(${PROJECT_NAME}
    ${GLFW_LIB}
    ${VULKAN_LIB}
    ${DL_LIB}
    ${PTHREAD_LIB}
    ${X11_LIB}
    ${XXF86VM_LIB}
    ${XRANDR_LIB}
    ${XI_LIB}
)
