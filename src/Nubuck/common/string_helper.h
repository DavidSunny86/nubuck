#pragma once

#include <string>

namespace COM {

std::string GetFileExtension(const std::string& str);
std::string StripFileExtension(const std::string& str);

} // namespace COM