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

Operator* Driver::ActiveOperator() {
    if(_activeOps.empty()) return NULL;
    return _activeOps.back();
}

void Driver::Event_Push(const EV::Event& event) {
    const ED::Params_Push& args = ED::def_Push.GetArgs(event);

	if(args.op->Invoke()) {
        Operator* op = ActiveOperator();
        if(op) op->Finish();

        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
	    g_operators.SendAndWait(event);
    } else {
        ED::Params_Push args;
        args.op = NULL; // indicates declined invocation
        g_operators.SendAndWait(ED::def_Push.Create(args));
    }

    SignalCompletion();
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnGeometrySelected();
    }

    SignalCompletion();
}

void Driver::Event_CameraChanged(const EV::Event& event) {
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnCameraChanged();
    }

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
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnEditModeChanged(W::editMode_t::Enum(args.editMode));
    }

    SignalCompletion();
}

void Driver::Event_Mouse(const EV::Event& event) {
	const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);
	bool shiftKey = args.mods & EV::Params_Mouse::MODIFIER_SHIFT;
    unsigned ret = 0;
    unsigned N;
    for(int i = _activeOps.size() - 1; !ret && 0 <= i; --i) {
        Operator* op = _activeOps[i];
		if(op->OnMouse(ConvertMouseEvent(args))) {
            N = _activeOps.size() - 1 - i;
            for(unsigned j = i; j < _activeOps.size(); ++j)
                _activeOps[j]->Finish();
            if(0 < N) op->Invoke();
            ret = 1;
        }
    }

    if(!ret) {
        // forward event
        W::world.Send(event);
    } else {
        // TODO: remove me
        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));

        if(0 < N) {
            ED::Params_Pop popArgs = { N };
            g_operators.SendAndWait(ED::def_Pop.Create(popArgs));
        }
    }
	event.Accept();

    SignalCompletion();
}

void Driver::Event_Key(const EV::Event& event) {
    const EV::Params_Key& args = EV::def_Key.GetArgs(event);
    for(int i = _activeOps.size() - 1; 0 <= i; --i) {
        Operator* op = _activeOps[i];
        op->OnKey(ConvertKeyEvent(args));
    }
    W::world.Send(event);

    SignalCompletion();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    Operator* op = ActiveOperator();
    if(op) {
        op->Send(event);
        op->HandleEvents();

        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
	}

    SignalCompletion();
}

Driver::Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx)
    : _isBlocked(false)
    , _activeOps(activeOps)
    , _activeOpsMtx(activeOpsMtx)
{
	AddEventHandler(ED::def_Push, this, &Driver::Event_Push);
	AddEventHandler(EV::def_SelectionChanged, this, &Driver::Event_SelectionChanged);
	AddEventHandler(EV::def_CameraChanged, this, &Driver::Event_CameraChanged);
    AddEventHandler(EV::def_EditModeChanged, this, &Driver::Event_EditModeChanged);
	AddEventHandler(EV::def_Mouse, this, &Driver::Event_Mouse);
    AddEventHandler(EV::def_Key, this, &Driver::Event_Key);
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