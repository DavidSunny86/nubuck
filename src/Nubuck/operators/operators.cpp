#include <QMessageBox>
#include <QDebug>

#include <nubuck_private.h>
#include <Nubuck\events\core_events.h>
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

void Operators::Event_SetOperator(const SetOperatorEvent& event) {
    // find panel of active operator
    OperatorPanel* panel = NULL;
    for(unsigned i = 0; !panel && i < _ops.size(); ++i) {
        if(_ops[i].op == event.m_op) panel = _ops[i].panel;
    }
    COM_assert(panel);

    panel->Invoke();
    g_ui.GetOperatorPanel().Clear();
    NB::SetOperatorPanel(panel->GetWidget());

    _panel = panel;

    event.Signal();
}

void Operators::Event_ShowConfirmationDialog(const EV::Event& event) {
    QMessageBox mb;
    mb.setText("Switch operators.");
    mb.setInformativeText(
        "This operation ends the currently active operator.\n"
        "Do you want to continue?");
    mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    mb.setIcon(QMessageBox::Question);
    int retval = mb.exec();
    if(QMessageBox::Ok == retval) event.SetReturnValue(1);
    else event.SetReturnValue(0);
    event.Signal();
}

void Operators::Event_ActionFinished(const EV::Event& event) {
    _actionsPending--;
}

void Operators::Event_ShowQuestionBox(const EV::ShowQuestionBox& event) {
    QMessageBox mb;
    mb.setText(QString::fromStdString(event.caption));
    mb.setInformativeText(QString::fromStdString(event.message));
    mb.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    mb.setIcon(QMessageBox::Question);
    int retval = mb.exec();
    if(QMessageBox::Ok == retval) event.SetReturnValue(1);
    else event.SetReturnValue(0);
    event.Signal();
}


// key events not handled in the operator, world are passed to the operators manager
void Operators::Event_Key(const EV::KeyEvent& event) {
    if(EV::KeyEvent::KEY_DOWN == event.type) {
        QKeySequence ks(event.mods + event.keyCode);
        qDebug() << "Operators::Event_Key, sequence = " << ks.toString();
        for(unsigned i = 0; i < _ops.size(); ++i) {
            if(!ks.isEmpty() && ks == _ops[i].shortcut) {
                _ops[i].invoker->OnInvoke();
            }
        }
    }
}

void Operators::OnInvokeOperator(unsigned id) {
    UI::LogWidget::Instance()->sys_printf("INFO - invoking operator with id = %d\n", id);

	if(!_driver.IsValid()) {
        _driver = GEN::MakePtr(new Driver(_ops[0].op)); // op_translate is first operator
        _driver->Thread_StartAsync();
	}

    Operator* op = _ops[id].op;
    SetOperatorEvent event(op, false);
    InvokeAction(ev_op_setOperator.Tag(event));
}

Operators::Operators() : _actionsPending(0), _panel(0) {
}

Operators::~Operators() {
    UnloadModules();
}

void Operators::Init() {
    AddEventHandler(ev_op_setOperator, this, &Operators::Event_SetOperator);
    AddEventHandler(ev_op_showConfirmationDialog, this, &Operators::Event_ShowConfirmationDialog);
    AddEventHandler(ev_op_actionFinished, this, &Operators::Event_ActionFinished);
    AddEventHandler(ev_ui_showQuestionBox, this, &Operators::Event_ShowQuestionBox);
    AddEventHandler(ev_key, this, &Operators::Event_Key);

    // forward other known events
    AddEventHandler(ev_w_editModeChanged, this, &Operators::Event_ForwardToDriver<EV::Arg<int> >);
    AddEventHandler(ev_w_selectionChanged, this, &Operators::Event_ForwardToDriver<EV::Event>);
    AddEventHandler(ev_usr_selectEntity, this, &Operators::Event_ForwardToDriver<EV::Usr_SelectEntity>);
    AddEventHandler(ev_usr_changeEditMode, this, &Operators::Event_ForwardToDriver<EV::Arg<int> >);
}

unsigned Operators::GetDriverQueueSize() const {
    return _driver->GetEventQueueSize();
}

/*
TODO: hacky hack hack
the operator order is defined by the calls to Register() in nubuck_main.cpp.
relative order of plugins is generally undefined.
replace this by name lookup sometimes
*/
Operator* Operators::GetOperatorByID(int id) {
    return _ops[id].op;
}

void Operators::FrameUpdate() {
    HandleEvents();

    if(_panel) _panel->HandleEvents();
}

unsigned Operators::Register(OperatorPanel* panel, Operator* op, HMODULE module) {
    unsigned id = _ops.size();

    // assign shortcut
    // TODO: check for duplicates
    QKeySequence shortcut = QKeySequence(QString::fromStdString(op->PreferredShortcut()));

    Invoker* invoker = new Invoker(id, shortcut);
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
    desc.shortcut = shortcut;

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