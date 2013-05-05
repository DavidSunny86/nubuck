#pragma once

#include <vector>
#include <map>

#include <generic\singleton.h>
#include "pass.h"

namespace R {

    struct EffectDesc {
        std::string             name;
        std::vector<PassDesc>   passes;
    };

    class Effect {
    private:
        const EffectDesc& _desc;

        std::vector<GEN::Pointer<R::Pass> > _passes;
    public:
        explicit Effect(const EffectDesc& desc);

        int     NumPasses(void) const;

        void    Compile(void);
        Pass*   GetPass(int id);
    };

    class EffectManager : public GEN::Singleton<EffectManager> {
        friend class GEN::Singleton<EffectManager>;
    private:
        std::map<std::string, GEN::Pointer<Effect> > _effects;
    public:
        void Add(const EffectDesc& desc);
        GEN::Pointer<Effect> GetEffect(const std::string& name);

        void FreeResources(void);
    };

    void CreateDefaultEffects(void);

} // namespace R