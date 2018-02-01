#pragma once
//
// FileUtil: Common file utilities for use at B2B
//
#include "b2btypes.h"

namespace FileUtil {
	// Returns true if the file with fileName exists, false otherwise.
	bool Exists(const char *fileName);
}
