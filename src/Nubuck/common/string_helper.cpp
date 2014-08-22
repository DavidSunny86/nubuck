#include <Nubuck\common\string_helper.h>
#include <common\string_helper.h>

namespace COM {

std::string GetFileExtension(const std::string& str) {
    using namespace std;
    string::size_type pos = str.find_last_of('.');
    if(string::npos == pos) return "";
    return str.substr(pos);
}

std::string StripFileExtension(const std::string& str) {
    using namespace std;
    string::size_type pos = str.find_last_of('.');
    return str.substr(0, pos);
}

// SDBM Hash, cnf. http://www.cse.yorku.ca/~oz/hash.html
NUBUCK_API unsigned StringHash(const char* str) {
    unsigned long hash = 0;
    int c;
    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

} // namespace COM