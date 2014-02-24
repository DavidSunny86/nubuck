#include <Windows.h>
#include <Nubuck\common\common.h>
#include <renderer\effects\nfx_loader\nfx_loader.h>
#include "effectmgr.h"
#include "effect.h"

namespace R {

    // processes all .nfx files in directory BASE\res\Effects
    static void CreateFromNFX(void) {
        std::string filename = common.BaseDir() + "Effects\\*";
        WIN32_FIND_DATAA ffd;
        HANDLE ff = FindFirstFileA(filename.c_str(), &ffd);
        if(INVALID_HANDLE_VALUE == ff) {
            if(ERROR_FILE_NOT_FOUND == GetLastError()) {
                common.printf("WARNING - directory %sres\\Effects does not exist.\n", common.BaseDir().c_str());
            } else {
                common.printf("CreateFromNFX: FindFirstFileA failed.\n");
                Crash();
            }
        }
        do {
            const char* ext = NULL;
            if((ext = strstr(ffd.cFileName, ".nfx")) && '\0' == ext[4]) {
                std::string fqname = common.BaseDir() + "Effects\\" + ffd.cFileName;
                EffectDesc desc;
                if(!NFX_Parse(fqname.c_str(), desc)) {
                    common.printf("ERROR - unable to load effect '%s'\n", fqname.c_str());
                    Crash();
                }
                effectMgr.Register(desc);
            }
        } while(FindNextFileA(ff, &ffd));
    }

    void CreateDefaultEffects(void) {
        CreateFromNFX();
    }

} // namespace R