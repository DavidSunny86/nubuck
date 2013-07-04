#include <common\common.h>
#include "config.h"

namespace COM {

    void Config::DumpVariables(void) {
        FILE* file = fopen("config_dump.txt", "w");
        if(file) {
            fprintf(file, "known variables:\n");
            VarInfo* varIt(_vars);
            while(varIt) {
                fprintf(file, "%s", varIt->Name().c_str());
                varIt = varIt->next;
            }
        }
        fclose(file);
    }

} // namespace COM