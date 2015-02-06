#pragma once

#include <Nubuck\nb_common.h>

namespace OP {

class VDH_Operator : public Operator {
private:
    NB::Mesh _cloudMesh;
    NB::Mesh _hullMesh;
public:
    VDH_Operator();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override;
};

} // namespace OP