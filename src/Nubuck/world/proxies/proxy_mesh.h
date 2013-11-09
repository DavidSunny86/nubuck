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
    void AlignZ(float x, float y, float z) override;
    void SetOrient(float x0, float y0, float z0, float x1, float y1, float z1) override;
    void SetEffect(const char* fxName) override;
    void SetVisible(bool isVisible) override;
};

} // namespace Proxy