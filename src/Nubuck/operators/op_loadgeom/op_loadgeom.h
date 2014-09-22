#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

class QLineEdit;

BEGIN_EVENT_DEF(OP_LoadGeom_Load)
    std::string* filename;
END_EVENT_DEF

namespace OP {

class LoadGeomPanel : public OperatorPanel {
    Q_OBJECT
private:
    QLineEdit* _leFilename;
private slots:
    void OnChooseFilename();
public:
    LoadGeomPanel(QWidget* parent = NULL);
};

class LoadGeom : public Operator {
private:
    void Event_Load(const EV::Event& event);
public:
    LoadGeom();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP