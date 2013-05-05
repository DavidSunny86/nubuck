#include <common\common.h>
#include "effect.h"

namespace R {

    Effect::Effect(const EffectDesc& desc) : _desc(desc) {
        typedef std::vector<PassDesc>::const_iterator pdescIt_t;

        for(pdescIt_t pdescIt(desc.passes.cbegin()); desc.passes.end() != pdescIt; ++pdescIt) {
            _passes.push_back(GEN::Pointer<Pass>(new Pass(*pdescIt)));
        }
    }

    int Effect::NumPasses(void) const { return _passes.size(); }

    void Effect::Compile(void) {
        typedef std::vector<GEN::Pointer<Pass> >::iterator passIt_t;

        for(passIt_t passIt(_passes.begin()); _passes.end() != passIt; ++passIt) {
            (*passIt)->Init();
        }
    }

    Pass* Effect::GetPass(int id) { return _passes[id].Raw(); }

    void EffectManager::Add(const EffectDesc& desc) {
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