#include <nubuck_private.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\animation\animator.h>
#include <UI\window_events.h>
#include <UI\userinterface.h>
#include <UI\logwidget\logwidget.h>
#include <world\world_events.h>
#include <world\world.h>
#include "operator_events.h"
#include "operator_driver.h"
#include "operators.h"

namespace OP {

Operators g_operators;

void Operators::Event_SetOperator(const EV::Arg<Operator*>& event) {
    // find panel of active operator
    OperatorPanel* panel = NULL;
    for(unsigned i = 0; !panel && i < _ops.size(); ++i) {
        if(_ops[i].op == event.value) panel = _ops[i].panel;
    }
    assert(panel);

    panel->Invoke();
    g_ui.GetOperatorPanel().Clear();
    g_nubuck.set_operator_panel(panel->GetWidget());

    _panel = panel;

    event.Accept();
}

void Operators::Event_ActionFinished(const EV::Event& event) {
    _actionsPending--;
}

void Operators::Event_ForwardToDriver(const EV::Event& event) {
    InvokeAction(event, InvokationMode::ALWAYS);
}

void Operators::OnInvokeOperator(unsigned id) {
    UI::LogWidget::Instance()->sys_printf("INFO - invoking operator with id = %d\n", id);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_ops[0].op)); // op_translate is first operator
        _driver->Thread_StartAsync();
	}

    Operator* op = _ops[id].op;
    InvokeAction(ev_op_setOperator.Tag(op));
}

Operators::Operators() : _actionsPending(0), _panel(0) {
}

Operators::~Operators() {
    UnloadModules();
}

void Operators::Init() {
    AddEventHandler(ev_op_setOperator, this, &Operators::Event_SetOperator);
    AddEventHandler(ev_op_actionFinished, this, &Operators::Event_ActionFinished);

    // forward other known events
    // AddEventHandler(ev_w_editModeChanged, this, &Operators::Event_ForwardToDriver); // URGENT
    AddEventHandler(ev_w_selectionChanged, this, &Operators::Event_ForwardToDriver);
}

unsigned Operators::GetDriverQueueSize() const {
    return _driver->GetEventQueueSize();
}

void Operators::FrameUpdate() {
    HandleEvents();

    if(_panel) _panel->HandleEvents();
}

unsigned Operators::Register(OperatorPanel* panel, Operator* op, HMODULE module) {
    unsigned id = _ops.size();

    Invoker* invoker = new Invoker(id);
    connect(invoker, SIGNAL(SigInvokeOperator(unsigned)), this, SLOT(OnInvokeOperator(unsigned)));

    if(!panel) panel = new OperatorPanel();

    op->SetPanel(panel);
    op->Register(*invoker);

    OperatorDesc desc;
    desc.id = id;
    desc.op = op;
    desc.invoker = invoker;
	desc.module = module;
    desc.panel = panel;

    _ops.push_back(desc);

    return id;
}

void Operators::InvokeAction(const EV::Event& event, InvokationMode::Enum mode) {
    A::g_animator.Filter(event);

    if(InvokationMode::DROP_WHEN_BUSY == mode && BUSY_THRESHOLD < _actionsPending) {
	    printf("INFO - Operators::InvokeAction: wait for driver, %d actions pending\n", _actionsPending);
        return;
    }

    _actionsPending++;
    _driver->Send(event);
}

void Operators::SetInitOp(unsigned id) {
    OnInvokeOperator(id);
}

// REMOVEME
void Operators::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
    // ...
}

NUBUCK_API void SendToOperator(const EV::Event& event) {
    g_operators.InvokeAction(event);
}

} // namespace OP