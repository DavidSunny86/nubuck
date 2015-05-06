#pragma once

#include <QtGui>
#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\UI\nbw_spinbox.h>

namespace OP {

namespace ParametricImpl {

enum {
    DEFAULT_SPHERE_SUBDIV = 1
};

struct Params : EV::Event {
    EVENT_TYPE(Params)

    int sphereSubdiv;
};

extern EV::ConcreteEventDef<Params> ev_paramsChanged;

} // namespace ParametricImpl 

class ParametricPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private:
    QStackedWidget* _paramStack;
    NBW_SpinBox*    _sphereSubdiv;

    QWidget* CreateSphereParamWidget();

    void SendUpdatedParams();
private slots:
    void OnSphereSubdivChanged(leda::rational value);
public:
    ParametricPanel();
};

class Parametric : public Operator {
private:
    int         _meshType; // in PMeshType::Enum
    NB::Mesh    _mesh;

    ParametricImpl::Params _params;

    void RebuildMesh();

    void Event_ParamsChanged(const ParametricImpl::Params& event);
public:
    Parametric();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override;
    bool IsDone() const override { return true; }
};

} // namespace OP