#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace helper
{
	uintptr_t find_pattern(const char* module_name, std::vector<unsigned char> pattern, uintptr_t start = 0);
	uintptr_t find_pattern(const char* module_name, const char* pattern, uintptr_t start = 0);

	std::vector<unsigned char> split_bytes(std::string string);

	void unmute_windows();

}
