cmake_minimum_required(VERSION 3.30)
project(learning C)

set(CMAKE_C_STANDARD 23)

set(PROJECT_EXTERNAL_DIR ${PROJECT_SOURCE_DIR}/external)

set(USR_INCLUDE /usr/include)
set(LINUX_X64_GNU_PATH /usr/lib/x86_64-linux-gnu)

set(VULKAN_SDK_DIR /opt/VULKAN_SDK/x86_64)
set(VULKAN_INCLUDE_DIR ${VULKAN_SDK_DIR}/include)

set(GLFW_DIR ${PROJECT_EXTERNAL_DIR}/glfw)
set(GLFW_INCLUDE_DIR ${GLFW_DIR}/include)

link_directories(${VULKAN_SDK_DIR}/lib)  # Adjust path if necessary
link_directories(${LINUX_X64_GNU_PATH})  # Adjust path if necessary
link_directories(${GLFW_DIR}/cmake-build-debug/src)  # Adjust path if necessary

include_directories(${USR_INCLUDE})
include_directories(${GLFW_INCLUDE_DIR})
include_directories(${VULKAN_INCLUDE_DIR})


add_executable(learning main.c)
target_link_libraries(learning glfw3 vulkan m)
