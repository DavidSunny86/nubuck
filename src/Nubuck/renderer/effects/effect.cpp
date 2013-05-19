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

} // namespace R