#pragma once
#include <zip.h>
#include <ctime>
#include <string>
#include <algorithm>

#include "core/detection/detection.hpp"

#ifdef _WIN32
#define COLOR_RED ""
#define COLOR_GREEN ""
#define COLOR_BLUE ""
#define COLOR_YELLOW ""
#define COLOR_RESET ""
#else
#define COLOR_RED "\e[0;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RESET "\033[0m"
#endif


// Source - https://stackoverflow.com/a/5253245
// Posted by Blastfurnace, modified by community. See post 'Timeline' for change history
// Retrieved 2026-02-03, License - CC BY-SA 2.5
inline std::string space2underscore(std::string text) {
    std::replace(text.begin(), text.end(), ' ', '_');
    return text;
}


inline std::string print_title()
{
    return R"(
  ██████  ▄▄▄    ██▒   █▓▓█████     ███▄ ▄███▓ ▄▄▄       ███▄    █  ▄▄▄        ▄████ ▓█████  ██▀███  
▒██    ▒ ▒████▄ ▓██░   █▒▓█   ▀    ▓██▒▀█▀ ██▒▒████▄     ██ ▀█   █ ▒████▄     ██▒ ▀█▒▓█   ▀ ▓██ ▒ ██▒
░ ▓██▄   ▒██  ▀█▄▓██  █▒░▒███      ▓██    ▓██░▒██  ▀█▄  ▓██  ▀█ ██▒▒██  ▀█▄  ▒██░▄▄▄░▒███   ▓██ ░▄█ ▒
  ▒   ██▒░██▄▄▄▄██▒██ █░░▒▓█  ▄    ▒██    ▒██ ░██▄▄▄▄██ ▓██▒  ▐▌██▒░██▄▄▄▄██ ░▓█  ██▓▒▓█  ▄ ▒██▀▀█▄  
▒██████▒▒ ▓█   ▓██▒▒▀█░  ░▒████▒   ▒██▒   ░██▒ ▓█   ▓██▒▒██░   ▓██░ ▓█   ▓██▒░▒▓███▀▒░▒████▒░██▓ ▒██▒
▒ ▒▓▒ ▒ ░ ▒▒   ▓▒█░░ ▐░  ░░ ▒░ ░   ░ ▒░   ░  ░ ▒▒   ▓▒█░░ ▒░   ▒ ▒  ▒▒   ▓▒█░ ░▒   ▒ ░░ ▒░ ░░ ▒▓ ░▒▓░
░ ░▒  ░ ░  ▒   ▒▒ ░░ ░░   ░ ░  ░   ░  ░      ░  ▒   ▒▒ ░░ ░░   ░ ▒░  ▒   ▒▒ ░  ░   ░  ░ ░  ░  ░▒ ░ ▒░
░  ░  ░    ░   ▒     ░░     ░      ░      ░     ░   ▒      ░   ░ ░   ░   ▒   ░ ░   ░    ░     ░░   ░ 
      ░        ░  ░   ░     ░  ░          ░         ░  ░         ░       ░  ░      ░    ░  ░   ░     
                     ░                                                                               
    )";
}

