#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

class QLineEdit;

BEGIN_EVENT_DEF(OP_SaveGeom_Save)
    std::string* filename;
END_EVENT_DEF

namespace OP {

class SaveGeomPanel : public OperatorPanel {
private:
    QLineEdit* _leFilename;

    void Event_Save(const EV::Event& event);
public:
    SaveGeomPanel(QWidget* parent = NULL);
    
    void Invoke() override;
};

class SaveGeom : public Operator {
private:
    Nubuck _nb;

    IGeometry* _geom;

    void Event_Save(const EV::Event& event);
public:
    SaveGeom();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP