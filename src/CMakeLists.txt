set(CORE_SOURCES
    graphics.cpp
    cert.cpp
    font.cpp
    log.cpp
    lang.cpp
    utils.cpp
    download_file.cpp
)

set(GUI_SOURCES
    gui/async_gui.cpp
    gui/download_file_gui.cpp
)

set(SOURCES ${CORE_SOURCES} ${GUI_SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(h8pks main.cpp ${SOURCES} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp ${MONGOOSE_DIR}/mongoose.c)
target_link_libraries(h8pks ${LIBRARIES})
target_compile_definitions(h8pks PUBLIC -DImTextureID=ImU64)