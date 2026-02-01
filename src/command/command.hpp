#pragma once
#include <zip.h>

#include "../detection/detection.hpp"
#include "../helpers/utils.hpp"

void handle_list(const Detection::DetectionResult& result);
void handle_backup(const Detection::DetectionResult& result);
void handle_restore(const Detection::DetectionResult& result);
