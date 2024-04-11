cmake_minimum_required(VERSION 3.14)
project(h8pks C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

if(NOT MSVC)
  if(${STATIC_LIBSTDCPP})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ ")
  endif()
endif()

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

# Mbedtls
set(MBEDTLS ./libraries/mbedtls)
option(ENABLE_PROGRAMS "Build mbed TLS programs." OFF)
option(ENABLE_TESTING "Build mbed TLS tests." OFF)
add_subdirectory(${MBEDTLS})
include_directories(${MBEDTLS}/include)

# Mongoose
set(MONGOOSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/mongoose)
include_directories(${MONGOOSE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMG_TLS=MG_TLS_MBED")

# 11zip
set(ELZIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/elzip)
set(ZLIB_TAG 3c13497a61c192dd255c280e73b806ed27ba8a0a)
set(RENAME_ZCONF OFF)
add_subdirectory(${ELZIP_DIR})

# Nlohmann Json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libraries/nlohmann_json)

# Native File Dialog Extended
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libraries/nativefiledialog-extended)

# Libraries
find_package(Vulkan REQUIRED)
set(LIBRARIES "glfw;Vulkan::Vulkan;MbedTLS::mbedtls;MbedTLS::mbedcrypto;MbedTLS::mbedx509;elzip;nlohmann_json::nlohmann_json;nfd")
if (WIN32)
    set (LIBRARIES "${LIBRARIES};ws2_32;iphlpapi")
endif()

# Use vulkan headers from glfw:
include_directories(${GLFW_DIR}/deps)

add_subdirectory(src)