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
    SaveGeomPanel(QWidget* parent = NULL);

    void Invoke() override;
};

class SaveGeom : public Operator {
private:
    nb::geometry _geom;

    void Event_Save(const EV::Arg<std::string*>& event);
public:
    SaveGeom();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP