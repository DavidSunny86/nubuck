#include <stdio.h>
#include <Nubuck\common\common.h>
#include <common\commands.h>
#include "config.h"

namespace COM {

namespace Impl {

bool Parse(const std::string& str, int& val) {
    return 1 == sscanf(str.c_str(), "%d", &val);
}

bool Parse(const std::string& str, float& val) {
    return 1 == sscanf(str.c_str(), "%f", &val);
}

void SPrint(char* buffer, unsigned bufSz, int val) {
    sprintf_s(buffer, bufSz, "%d", val);
}

void SPrint(char* buffer, unsigned bufSz, float val) {
    sprintf_s(buffer, bufSz, "%f", val);
}

} // namespace Impl

void Config::DumpVariables(void) {
    FILE* file = fopen("config_dump.txt", "w");
    if(file) {
        fprintf(file, "known variables:\n");
        VarInfo* varIt(_vars);
        while(varIt) {
            fprintf(file, "%s\n", varIt->Name().c_str());
            varIt = varIt->next;
        }
    }
    fclose(file);
}

Config::variable_t Config::FirstVariable() {
    return _vars;
}

Config::variable_t Config::NextVariable(variable_t var) {
    return var->next;
}

const char* Config::TypeString(variable_t var) {
    return var->TypeString();
}

const std::string& Config::Name(variable_t var) {
    return var->Name();
}

std::string Config::ValueString(variable_t var) {
    char buffer[512];
    memset(buffer, 0, sizeof(buffer));
    var->SPrint(buffer, 512);
    return std::string(buffer);
}

bool Config::Parse(variable_t var, const std::string& str) {
    return var->Parse(str);
}

// ==================================================
// COMMANDS
// ==================================================

void CMD_ListVariables(QTextStream& cout, const char*) {
    Config& cfg = Config::Instance();
    Config::variable_t var = cfg.FirstVariable();
    while(var) {
        cout << cfg.Name(var).c_str() << " = " << cfg.ValueString(var).c_str() <<  "\n";
        var = cfg.NextVariable(var);
    }
}

void CMD_SetVariable(QTextStream& cout, const char* args) {
    Config& cfg = Config::Instance();

    COM::ItTokenizer toks(args, " \t");
    COM::ItTokenizer::Token tok;

    tok = toks.NextToken();
    const std::string name = std::string(tok.start, toks.Length(tok));

    Config::variable_t var = cfg.FirstVariable();
    while(var && name != cfg.Name(var)) {
        var = cfg.NextVariable(var);
    }

    if(var) {
        tok = toks.NextToken();
        const std::string valueStr = std::string(tok.start);

        if(!cfg.Parse(var, valueStr)) {
            cout
                << "error parsing \"" << valueStr.c_str() << "\" "
                << "as type \"" << cfg.TypeString(var) << "\""
                << "\n";
        }
    } else {
        cout << "unknown variable \"" << name.c_str() << "\"\n";
    }
}

} // namespace COM