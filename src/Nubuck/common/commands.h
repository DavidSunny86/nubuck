#pragma once

#include <QTextStream>
#include <string>

namespace COM {
namespace CMD {

typedef void (*callback_t)(QTextStream& cout, const char* args);

void RegisterCommand(const std::string& name, const std::string& desc, callback_t callback);
void ExecCommand(QTextStream& cout, const char* cmd);

} // namespace CMD
} // namespace COM