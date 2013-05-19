#pragma once

#include <map>
#include <string>
#include "effect.h"

namespace R {

	class EffectManager : private GEN::Uncopyable {
    private:
        std::map<std::string, GEN::Pointer<Effect> > _effects;
    public:
        void Register(const EffectDesc& desc);

        GEN::Pointer<Effect> GetEffect(const std::string& name);

        void FreeResources(void);
    };

	extern EffectManager effectMgr;

} // namespace R