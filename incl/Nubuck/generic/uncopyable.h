#pragma once

#include <Nubuck\nubuck_api.h>

namespace GEN {

    // NOTE: Uncopyable has to be exported because other exported
    // classes derive from it
    class NUBUCK_API Uncopyable {
    private:
        Uncopyable(const Uncopyable& other);
        Uncopyable& operator=(const Uncopyable& other);
    public:
        Uncopyable(void) { }
        ~Uncopyable(void) { }
    };

} // namespace GEN
