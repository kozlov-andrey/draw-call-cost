#pragma once

#include <functional>

void collect_statistics(std::function<void()> swap_func, int width, int height, const char *output_path);