#pragma once

#include <assert.h>

#include <QObject>

#include <vector>
#include <Nubuck\operators\operator.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <events\events.h>
#include <renderer\renderer.h>
#include <system\thread\thread.h>

namespace OP {

class Driver;

class Operators : public QObject, public EV::EventHandler<> {
    Q_OBJECT
private:
    DECL_HANDLE_EVENTS(Operators)

    struct OperatorDesc {
        unsigned    id;
        Operator*   op;
        Invoker*    invoker;
        QWidget*    panel;
        HMODULE     module;
    };

    std::vector<OperatorDesc>   _ops;       // all registered operators
    std::vector<Operator*>      _activeOps; // active operators
    SYS::SpinLock               _activeOpsMtx;
    unsigned                    _actionsPending;

    GEN::Pointer<Driver>        _driver;

    bool IsActiveOperatorBusy() const { return 0 <_actionsPending; }

    void UnloadModules();

    void Event_ActionFinished(const EV::Event& event);
public slots:
    void OnInvokeOperator(unsigned id);
public:
    Operators();
    ~Operators();

    void FrameUpdate();

    unsigned Register(QWidget* panel, Operator* op, HMODULE module = NULL);

    void InvokeAction(const EV::Event& event);

    void SetInitOp(unsigned id);

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs);

    void SelectGeometry();

    void OnCameraChanged();
    bool OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey);
    bool OnMouseMove(const M::Vector2& mouseCoords);
    bool OnMouseUp(const M::Vector2& mouseCoords);
};

extern Operators g_operators;

} // namespace OP