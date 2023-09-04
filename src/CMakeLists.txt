find_package(Vulkan REQUIRED)

set(CORE_SOURCES

)

add_executable(imgui_glfw_vulkan_template main.cpp ${CORE_SOURCES} ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp ${IMGUI_DIR}/imgui.cpp ${IMGUI_DIR}/imgui_draw.cpp ${IMGUI_DIR}/imgui_demo.cpp ${IMGUI_DIR}/imgui_tables.cpp ${IMGUI_DIR}/imgui_widgets.cpp)
target_link_libraries(imgui_glfw_vulkan_template ${LIBRARIES})
target_compile_definitions(imgui_glfw_vulkan_template PUBLIC -DImTextureID=ImU64)