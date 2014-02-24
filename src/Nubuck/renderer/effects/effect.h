#pragma once

#include <vector>
#include <map>

#include <Nubuck\generic\singleton.h>
#include "pass.h"

namespace R {

    struct EffectDesc {
        unsigned                sortKey;
        std::string             name;
        std::vector<PassDesc>   passes;
    };

    class Effect {
    private:
        EffectDesc _desc;

        std::vector<GEN::Pointer<R::Pass> > _passes;
    public:
        explicit Effect(const EffectDesc& desc);

        unsigned    SortKey(void) const;
        int         NumPasses(void) const;

        void    Compile(void);
        Pass*   GetPass(int id);
    };

    void CreateDefaultEffects(void);

} // namespace R