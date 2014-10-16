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
    g_operators.Send(ED::def_ActionFinished.Create(ED::Params_ActionFinished()));
}

void Driver::Event_SetOperator(const EV::Event& event) {
    const ED::Params_SetOperator& args = ED::def_SetOperator.GetArgs(event);

	if(args.op->Invoke()) {
        if(_activeOp) _activeOp->Finish();
        _activeOp = args.op;

        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
	    g_operators.SendAndWait(event);
    }

    SignalCompletion();
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    if(_activeOp) _activeOp->OnGeometrySelected();
    SignalCompletion();
}

void Driver::Event_CameraChanged(const EV::Event& event) {
    if(_activeOp) _activeOp->OnCameraChanged();
    SignalCompletion();
}

static MouseEvent ConvertMouseEvent(const EV::Params_Mouse& from) {
    MouseEvent to;
	to.type     = MouseEvent::Type(from.type);
	to.button   = MouseEvent::Button(from.button);
	to.mods     = from.mods;
	to.delta    = from.delta;
	to.coords   = M::Vector2(from.x, from.y);
    return to;
}

static KeyEvent ConvertKeyEvent(const EV::Params_Key& from) {
    KeyEvent to;
    to.type             = from.type;
    to.keyCode          = from.keyCode;
    to.nativeScanCode   = from.nativeScanCode;
    to.autoRepeat       = from.autoRepeat;
    return to;
}

void Driver::Event_EditModeChanged(const EV::Event& event) {
    const EV::Params_EditModeChanged& args = EV::def_EditModeChanged.GetArgs(event);
    if(_activeOp) _activeOp->OnEditModeChanged(W::editMode_t::Enum(args.editMode));
    SignalCompletion();
}

void Driver::Event_Mouse(const EV::Event& event) {
	const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);
	bool shiftKey = args.mods & EV::Params_Mouse::MODIFIER_SHIFT;
	if(_activeOp && _activeOp->OnMouse(ConvertMouseEvent(args))) {
        // TODO: remove me
        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
    } else {
        // forward event
        W::world.Send(event);
    }
	event.Accept();
    SignalCompletion();
}

void Driver::Event_Key(const EV::Event& event) {
    const EV::Params_Key& args = EV::def_Key.GetArgs(event);
    if(_activeOp) _activeOp->OnKey(ConvertKeyEvent(args));
    W::world.Send(event);

    SignalCompletion();
}

void Driver::Event_RebuildAll(const EV::Event& event) {
    W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
    SignalCompletion();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    if(_activeOp) {
        _activeOp->Send(event);
        _activeOp->HandleEvents();

        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
	}

    SignalCompletion();
}

Driver::Driver()
    : _isBlocked(false)
    , _activeOp(0)
{
	AddEventHandler(ED::def_SetOperator, this, &Driver::Event_SetOperator);
	AddEventHandler(EV::def_SelectionChanged, this, &Driver::Event_SelectionChanged);
	AddEventHandler(EV::def_CameraChanged, this, &Driver::Event_CameraChanged);
    AddEventHandler(EV::def_EditModeChanged, this, &Driver::Event_EditModeChanged);
	AddEventHandler(EV::def_Mouse, this, &Driver::Event_Mouse);
    AddEventHandler(EV::def_Key, this, &Driver::Event_Key);
    AddEventHandler(EV::def_RebuildAll, this, &Driver::Event_RebuildAll);
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