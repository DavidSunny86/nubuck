#pragma once

#include <vector>
#include <string>
#include <renderer\shader\shader.h>

namespace R {
namespace SPP {

bool PreprocessShaderSource(const std::string& in, std::string& out, std::vector<AttributeLocation>& attribLocs);

} // namespace SPP

} // namespace R