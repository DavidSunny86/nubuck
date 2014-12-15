#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>

class QLineEdit;

namespace OP {

class LoadGeomPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private:
    QLineEdit* _leFilename;
private slots:
    void OnChooseFilename();
public:
    LoadGeomPanel();
};

class LoadGeom : public Operator {
private:
    void Event_Load(const EV::Arg<std::string*>& event);
public:
    LoadGeom();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP