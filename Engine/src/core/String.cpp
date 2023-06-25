#include <string.h>
#include "String.hpp"

bool String::StringCompare(const char* str0, const char* str1) {
	return strcmp(str0, str1) == 0;
}