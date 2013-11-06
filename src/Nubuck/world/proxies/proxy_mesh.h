#pragma once

#include <Nubuck\nubuck.h>
#include <renderer\mesh\mesh.h>

namespace Proxy {

class Mesh : public IMesh {
private:
    unsigned _entId;
public:
    Mesh(unsigned entId) : _entId(entId) { }

    void SetPosition(float x, float y, float z) override;
    void SetScale(float sx, float sy, float sz) override;
    void SetEffect(const char* fxName) override;
    void SetVisible(bool isVisible) override;
};

} // namespace Proxy