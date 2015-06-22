#pragma once

#include <assert.h>

#include <QObject>

#include <vector>
#include <Nubuck\operators\operator.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <Nubuck\events\events.h>
#include <renderer\renderer.h>
#include <system\thread\thread.h>
#include <Nubuck\system\locks\scoped_lock.h>

// forward declarations

struct SetOperatorEvent;

namespace EV {

struct ShowQuestionBox;

} // namespace EV

namespace OP {

class Driver;

class Operators : public QObject, public EV::EventHandler<> {
    Q_OBJECT
private:
    DECL_HANDLE_EVENTS(Operators)

    struct OperatorDesc {
        unsigned        id;
        Operator*   	op;
        Invoker*    	invoker;
        OperatorPanel*  panel;
        HMODULE         module;
        QKeySequence    shortcut;
    };

    std::vector<OperatorDesc>   _ops; // all registered operators

    enum { BUSY_THRESHOLD = 24 };                   // driver is considered busy iff threshold < actionsPending
    unsigned                    _actionsPending;    // number of pending actions sent to the driver

    GEN::Pointer<Driver>        _driver;
    OperatorPanel*              _panel;

    void UnloadModules();

    void Event_SetOperator(const SetOperatorEvent& event);
    void Event_ShowConfirmationDialog(const EV::Event& event);
    void Event_ActionFinished(const EV::Event& event);

    void Event_ShowQuestionBox(const EV::ShowQuestionBox& event);

    void Event_Key(const EV::KeyEvent& event);

    template<typename T> void Event_ForwardToDriver(const T& event) {
        InvokeAction(event, InvokationMode::ALWAYS);
    }
public slots:
    void OnInvokeOperator(unsigned id);
public:
    Operators();
    ~Operators();

    void Init();

    unsigned GetDriverQueueSize() const;

    unsigned IsDriverIdle() const { return 0 == _actionsPending; }

    Driver& GetDriver() { assert(_driver.IsValid()); return *_driver; }

    Operator* GetDefaultOperator() { return _ops[0].op; }

    void FrameUpdate();

    unsigned Register(OperatorPanel* panel, Operator* op, HMODULE module = NULL);

    struct InvokationMode {
        enum Enum {
            ALWAYS = 0,
            DROP_WHEN_BUSY
        };
    };

    void InvokeAction(const EV::Event& event, InvokationMode::Enum mode = InvokationMode::DROP_WHEN_BUSY);

    void SetInitOp(unsigned id);

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs);
};

void LoadOperators();

extern Operators g_operators;

} // namespace OP