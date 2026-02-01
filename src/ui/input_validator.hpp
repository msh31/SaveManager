#pragma once

#include <string>
#include <iostream>
#include <limits>

int get_int(const std::string& prompt, int min, int max);
int get_int(const std::string& prompt = "");
void clear_input_error();
