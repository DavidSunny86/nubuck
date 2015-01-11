#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

class QLineEdit;

namespace OP {

class SaveGeomPanel : public OperatorPanel {
private:
    QLineEdit* _leFilename;

    void Event_Save(const EV::Arg<std::string*>& event);
public:
    SaveGeomPanel();

    void Invoke() override;
};

class SaveGeom : public Operator {
private:
    NB::Mesh _mesh;

    void Event_Save(const EV::Arg<std::string*>& event);
public:
    SaveGeom();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP