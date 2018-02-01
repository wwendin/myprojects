//
// FileUtil: Common file utilities for use at B2B
//
#include <sys/stat.h>

#include "FileUtil.h"

bool FileUtil::Exists(const char *fileName)
{
	// Could use ifstream and good() but this is faster
	struct stat buf;
	return stat(fileName, &buf) == 0;
}
