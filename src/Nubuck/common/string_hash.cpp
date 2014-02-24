#include <Nubuck\common\string_hash.h>

namespace COM {

// SDBM Hash, cnf. http://www.cse.yorku.ca/~oz/hash.html
unsigned StringHash(const char* str) {
    unsigned long hash = 0;
    int c;
    while (c = *str++)
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}

} // namespace COM