#include <Nubuck\operators\operator.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
#include <UI\window_events.h>
#include "operator_events.h"
#include "operators.h"
#include "operator_driver.h"

namespace OP {

// IMPORTANT: must be called for every event
inline void SignalCompletion() {
    g_operators.Send(ev_op_actionFinished, EV::Event());
}

void Driver::SetOperator(Operator* op) {
    if(_activeOp == op) return;

	if(op->Invoke()) {
        if(_activeOp) _activeOp->Finish();
        _activeOp = op;

        W::world.SendAndWait(ev_w_rebuildAll, EV::Event());

        EV::Arg<Operator*> event(op);
        g_operators.SendAndWait(ev_op_setOperator, event);
    }
}

void Driver::Event_SetOperator(const EV::Arg<Operator*>& event) {
    SetOperator(event.value);
    SignalCompletion();
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    if(_activeOp) _activeOp->OnGeometrySelected();
    _defaultOp->OnGeometrySelected();
    SignalCompletion();
}

static MouseEvent ConvertMouseEvent(const EV::MouseEvent& from) {
    MouseEvent to;
	to.type     = MouseEvent::Type(from.type);
	to.button   = MouseEvent::Button(from.button);
	to.mods     = from.mods;
	to.delta    = from.delta;
	to.coords   = M::Vector2(from.x, from.y);
    return to;
}

static KeyEvent ConvertKeyEvent(const EV::KeyEvent& from) {
    KeyEvent to;
    to.type             = from.type;
    to.keyCode          = from.keyCode;
    to.nativeScanCode   = from.nativeScanCode;
    to.autoRepeat       = from.autoRepeat;
    return to;
}

void Driver::Event_EditModeChanged(const EV::Arg<int>& event) {
    const W::editMode_t::Enum editMode = W::editMode_t::Enum(event.value);
    if(_activeOp) _activeOp->OnEditModeChanged(editMode);
    _defaultOp->OnEditModeChanged(editMode);
    SignalCompletion();
}

void Driver::Event_Mouse(const EV::MouseEvent& event) {
    const MouseEvent mouseEvent = ConvertMouseEvent(event);
	if(_activeOp && _activeOp->OnMouse(mouseEvent)) {
    } else if(_defaultOp->OnMouse(mouseEvent)) {
        // default operator becomes active, implicit rebuild
        SetOperator(_defaultOp);
    }
    W::world.SendAndWait(ev_w_rebuildAll, EV::Event());
	event.Accept();
    SignalCompletion();
}

void Driver::Event_Key(const EV::KeyEvent& event) {
    const KeyEvent keyEvent = ConvertKeyEvent(event);
    if(_activeOp && _activeOp->OnKey(keyEvent)) {
    } else if(_defaultOp->OnKey(keyEvent)) {
        // default operator becomes active, implicit rebuild
        SetOperator(_defaultOp);
    } else {
        // forward event
        W::world.Send(event);
    }

    W::world.SendAndWait(ev_w_rebuildAll, EV::Event());
    SignalCompletion();
}

void Driver::Event_RebuildAll(const EV::Event& event) {
    W::world.SendAndWait(ev_w_rebuildAll, EV::Event());
    SignalCompletion();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    if(_activeOp) {
        _activeOp->Send(event);
        _activeOp->HandleEvents();

        W::world.SendAndWait(ev_w_rebuildAll, EV::Event());
	}

    SignalCompletion();
}

Driver::Driver(Operator* defaultOp)
    : _isBlocked(false)
    , _defaultOp(defaultOp)
    , _activeOp(0)
{
    assert(_defaultOp);

	AddEventHandler(ev_op_setOperator, this, &Driver::Event_SetOperator);
	AddEventHandler(ev_w_selectionChanged, this, &Driver::Event_SelectionChanged);
    AddEventHandler(ev_w_editModeChanged, this, &Driver::Event_EditModeChanged);
	AddEventHandler(ev_mouse, this, &Driver::Event_Mouse);
    AddEventHandler(ev_key, this, &Driver::Event_Key);
    AddEventHandler(ev_w_rebuildAll, this, &Driver::Event_RebuildAll);
}

bool Driver::IsBlocked() const {
    return _isBlocked;
}

void Driver::Wait(SYS::ConditionVariable& cvar, bool (*testFunc)()) {
    SYS::SpinLock mtx;
    mtx.Lock();
    _isBlocked = true;
    while(!testFunc()) {
        cvar.Wait(mtx);
    }
    _isBlocked = false;
    mtx.Unlock();
}

DWORD Driver::Thread_Func() {
    while(true) {
        HandleEvents();
    }
}

} // namespace OP