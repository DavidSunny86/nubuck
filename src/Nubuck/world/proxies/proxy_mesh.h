#pragma once

#include <Nubuck\nubuck.h>
#include <renderer\mesh\mesh.h>

namespace Proxy {

class Mesh : public IMesh {
private:
    unsigned _entId;
public:
    Mesh(unsigned entId) : _entId(entId) { }

    void SetEffect(const char* fxName) override;
    void SetVisible(bool isVisible) override;
};

} // namespace Proxy