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

    void CreateDefaultEffects(void);

} // namespace R