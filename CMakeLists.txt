cmake_minimum_required(VERSION 3.14)
project(h8pks C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

# Vulkan
find_package(Vulkan REQUIRED)

# GLFW
set(GLFW_DIR ./libraries/glfw) # Set this to point to an up-to-date GLFW repo
option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
option(GLFW_INSTALL "Generate installation target" OFF)
option(GLFW_DOCUMENT_INTERNALS "Include internals in documentation" OFF)
add_subdirectory(${GLFW_DIR} binary_dir EXCLUDE_FROM_ALL)
include_directories(${GLFW_DIR}/include)
include_directories(${GLFW_DIR}/deps) # Use vulkan headers from glfw:

# Dear ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/imgui)
include_directories(${IMGUI_DIR} ${IMGUI_DIR}/backends ..)
set(IMGUI_SOURCES
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
)

# Mbedtls
set(MBEDTLS ./libraries/mbedtls)
option(ENABLE_PROGRAMS "Build mbed TLS programs." OFF)
option(ENABLE_TESTING "Build mbed TLS tests." OFF)
add_subdirectory(${MBEDTLS})
include_directories(${MBEDTLS}/include)

# Mongoose
set(MONGOOSE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libraries/mongoose)
include_directories(${MONGOOSE_DIR})
set(MONGOOSE_SOURCES
  ${MONGOOSE_DIR}/mongoose.c
)

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

# h8pks src files
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(${PROJECT_NAME} ${H8PKS_SOURCES} ${IMGUI_SOURCES} ${MONGOOSE_SOURCES})

target_link_libraries(${PROJECT_NAME}
  glfw
  Vulkan::Vulkan
  MbedTLS::mbedtls
  MbedTLS::mbedcrypto
  MbedTLS::mbedx509
  elzip
  nlohmann_json::nlohmann_json
  nfd
  $<$<PLATFORM_ID:Windows>:ws2_32>
  $<$<PLATFORM_ID:Windows>:iphlpapi>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
  ImTextureID=ImU64  # ImGui
  MG_TLS=MG_TLS_MBED # Mongoose
)

add_custom_target(h8pks_assets ALL COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:h8pks>/assets)
add_custom_command(TARGET h8pks_assets POST_BUILD COMMAND ${CMAKE_COMMAND} -E echo "Copy assets to $<TARGET_FILE_DIR:h8pks>")
