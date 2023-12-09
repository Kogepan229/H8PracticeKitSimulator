set(CORE_SOURCES
    graphics.cpp
    font.cpp
    log.cpp
    lang.cpp
    utils.cpp
    entity.cpp
    emulator/emulator.cpp
)

set(NETWORK_SOURCES
    network/cert.cpp
    network/http_get.cpp
    network/download_file.cpp
)

set(GUI_SOURCES
    gui/download_emulator_gui.cpp
)

set(H8PKS_SOURCES ${CORE_SOURCES} ${NETWORK_SOURCES} ${GUI_SOURCES})

set(IMGUI_SOURCES
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(h8pks main.cpp ${H8PKS_SOURCES} ${IMGUI_SOURCES} ${MONGOOSE_DIR}/mongoose.c)
target_link_libraries(h8pks ${LIBRARIES})
target_compile_definitions(h8pks PUBLIC -DImTextureID=ImU64)