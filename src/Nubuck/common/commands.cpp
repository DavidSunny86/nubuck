#include <algorithm>
#include <vector>
#include <Nubuck\common\common.h>
#include <common\commands.h>

namespace COM {
namespace CMD {

struct Command {
    std::string name;
    std::string desc;
    callback_t  callback;

    Command(const std::string& name, const std::string& desc, callback_t callback)
        : name(name)
        , desc(desc)
        , callback(callback)
    { }
};

std::vector<Command> cmds;

struct FindNamePred {
    const std::string& name;
    FindNamePred(const std::string& name) : name(name) { }
    bool operator()(const Command& cmd) const { return name == cmd.name; }
};

void RegisterCommand(const std::string& name, const std::string& desc, callback_t callback) {
    cmds.push_back(Command(name, desc, callback));
}

void ExecCommand(QTextStream& cout, const char* cmd) {
    COM::ItTokenizer toks(cmd, " \t");
    COM::ItTokenizer::Token tok = toks.NextToken();
    const std::string name = std::string(tok.start, toks.Length(tok));
    const FindNamePred pred = FindNamePred(name);
    std::vector<Command>::iterator it = std::find_if(cmds.begin(), cmds.end(), pred);
    if(cmds.end() == it) {
        cout << "unknown command '" << name.c_str() << "'\n";
        return;
    }
    tok = toks.NextToken();
    cout << cmd << ": ";
    it->callback(cout, tok.start);
}

} // namespace CMD
} // namespace COM