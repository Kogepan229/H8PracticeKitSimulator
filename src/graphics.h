#pragma once

#include <string>

#include "imgui.h"

namespace graphics {
namespace init {

int init_window(std::string window_name);
void init_imgui();
void upload_fonts();

}  // namespace init

void resize_swap_chain();
bool window_should_close();
void render(ImVec4 clear_color);
void cleanup();

}  // namespace graphics