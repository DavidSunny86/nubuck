#include <common\common.h>
#include "effectmgr.h"

namespace R {

	EffectManager effectMgr;

	void EffectManager::Register(const EffectDesc& desc) {
        typedef std::map<std::string, GEN::Pointer<Effect> >::iterator fxIt_t;

        fxIt_t fxIt(_effects.find(desc.name));
        if(_effects.end() != fxIt) {
            common.printf("WARNING - attempting to add already existing effect named '%s'.\n", desc.name.c_str());
            return;
        }
        _effects[desc.name] = GEN::Pointer<Effect>(new Effect(desc));
    }

    GEN::Pointer<Effect> EffectManager::GetEffect(const std::string& name) {
        typedef std::map<std::string, GEN::Pointer<Effect> >::iterator fxIt_t;

        fxIt_t fxIt(_effects.find(name));
        if(_effects.end() == fxIt) {
            common.printf("ERROR - unable to find effect '%s'.\n", name.c_str());
            Crash();
            return GEN::Pointer<Effect>();
        }

        return fxIt->second;
    }

    void EffectManager::FreeResources(void) {
        _effects.clear();
    }

} // namespace R
