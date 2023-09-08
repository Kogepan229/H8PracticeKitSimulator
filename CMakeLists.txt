cmake_minimum_required(VERSION 3.5)
project(h8pks C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVK_PROTOTYPES  -pipe -fintegrated-as -fintegrated-cc1")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -ftrivial-auto-var-init=pattern -Wtautological-compare -Wsign-compare")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_PROTOTYPES -pipe -fintegrated-as -fintegrated-cc1")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra -O1 -ftrivial-auto-var-init=pattern -Wtautological-compare -Wsign-compare")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3")

if(WIN32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=lld")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=lld")
endif()

set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} -flto=full -fwhole-program-vtables -fforce-emit-vtables -fvirtual-function-elimination -ffunction-sections -fdata-sections -Wl,--gc-sections")

# GLFW
set(GLFW_DIR ./libraries/glfw) # Set this to point to an up-to-date GLFW repo
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)

# Dear ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)

# Mongoose
set(MONGOOSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/mongoose)
include_directories(${MONGOOSE_DIR})

# Libraries
find_package(Vulkan REQUIRED)
set(LIBRARIES "glfw;Vulkan::Vulkan")
if (WIN32)
    set (LIBRARIES "${LIBRARIES};ws2_32")
endif()

# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)

add_subdirectory(src)