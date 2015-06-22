#pragma once

#include <Nubuck\operators\operator.h>
#include <world\entities\ent_geometry\ent_geometry_events.h>

namespace OP {

class SetTransformPanel : public OperatorPanel {
};

class SetTransform : public Operator {
private:
    void Event_EntUsrSetPosition(const SetEntityVectorEvent& event);
public:
    SetTransform();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP