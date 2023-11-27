#pragma once

#include <string>

#include "imgui.h"

namespace graphics {
namespace init {

int init_window(std::string window_name);
void init_imgui();

}  // namespace init

bool window_should_close();
void new_frame();
void render(ImVec4 clear_color);
void cleanup();

}  // namespace graphics