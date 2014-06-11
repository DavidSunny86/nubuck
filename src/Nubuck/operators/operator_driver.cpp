#include <Nubuck\operators\operator.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
#include <UI\window_events.h>
#include "operator_events.h"
#include "operators.h"
#include "operator_driver.h"

namespace OP {

Operator* Driver::ActiveOperator() {
    if(_activeOps.empty()) return NULL;
    return _activeOps.back();
}

void Driver::Event_Push(const EV::Event& event) {
	const EV::Params_OP_Push& args = EV::def_OP_Push.GetArgs(event);

	args.op->Invoke();

    Operator* op = ActiveOperator();
    if(op) op->Finish();

    W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));
	g_operators.SendAndWait(event);
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnGeometrySelected();
    }
}

void Driver::Event_CameraChanged(const EV::Event& event) {
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnCameraChanged();
    }
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
            EV::Params_OP_Pop popArgs = { N };
            g_operators.SendAndWait(EV::def_OP_Pop.Create(popArgs));
        }
    }
	event.Accept();
}

void Driver::Event_Key(const EV::Event& event) {
    const EV::Params_Key& args = EV::def_Key.GetArgs(event);
    for(int i = _activeOps.size() - 1; 0 <= i; --i) {
        Operator* op = _activeOps[i];
        op->OnKey(ConvertKeyEvent(args));
    }
    W::world.Send(event);
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    Operator* op = ActiveOperator();
    if(op) {
        op->Send(event);
        op->HandleEvents();

        W::world.SendAndWait(EV::def_RebuildAll.Create(EV::Params_RebuildAll()));

		g_operators.Send(EV::def_OP_ActionFinished.Create(EV::Params_OP_ActionFinished()));
	}
}

Driver::Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx) 
    : _activeOps(activeOps)
    , _activeOpsMtx(activeOpsMtx)
{ 
	AddEventHandler(EV::def_OP_Push, this, &Driver::Event_Push);
	AddEventHandler(EV::def_SelectionChanged, this, &Driver::Event_SelectionChanged);
	AddEventHandler(EV::def_CameraChanged, this, &Driver::Event_CameraChanged);
    AddEventHandler(EV::def_EditModeChanged, this, &Driver::Event_EditModeChanged);
	AddEventHandler(EV::def_Mouse, this, &Driver::Event_Mouse);
    AddEventHandler(EV::def_Key, this, &Driver::Event_Key);
}

DWORD Driver::Thread_Func() {
    while(true) { 
        HandleEvents(); 
    }
}

} // namespace OP