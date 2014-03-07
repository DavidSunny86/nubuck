#include <Nubuck\operators\operator.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
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
    SYS::ScopedLock lock(_activeOpsMtx);
	const EV::Params_OP_Push& args = EV::def_OP_Push.GetArgs(event);
    Operator* op = ActiveOperator();
    if(op) {
        op->Finish();
        if(1 < _activeOps.size()) _activeOps.pop_back();
    }
	_activeOps.push_back(args.op);
	args.op->Invoke();
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    SYS::ScopedLock lock(_activeOpsMtx);
    for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
        _activeOps.rend() != it; ++it)
    {
        (*it)->OnGeometrySelected();
    }
}

void Driver::Event_CameraChanged(const EV::Event& event) {
	SYS::ScopedLock lock(_activeOpsMtx);
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

void Driver::Event_Mouse(const EV::Event& event) {
    SYS::ScopedLock lock(_activeOpsMtx);
	const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);
	bool shiftKey = args.mods & EV::Params_Mouse::MODIFIER_SHIFT;
    unsigned ret = 0;
    for(int i = _activeOps.size() - 1; !ret && 0 <= i; --i) {
        Operator* op = _activeOps[i];
		if(op->OnMouse(ConvertMouseEvent(args))) {
            unsigned N = _activeOps.size() - 1 - i;
            for(unsigned j = 0; j < N; ++j) {
                _activeOps.back()->Finish();
                _activeOps.pop_back();
            }
            if(i != _activeOps.size() - 1) op->Invoke();
			*args.ret = 1;
        }
    }
	event.Accept();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    SYS::ScopedLock lock(_activeOpsMtx);
    Operator* op = ActiveOperator();
    if(op) {
        op->Send(event);
        op->HandleEvents();

		g_operators.Send(EV::def_OP_ActionFinished.Create(EV::Params_OP_ActionFinished()));
	}
}

Driver::Driver(
	std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx,
    std::vector<R::MeshJob>& meshJobs, SYS::SpinLock& meshJobsMtx) 
	:   _activeOps(activeOps), _activeOpsMtx(activeOpsMtx),
        _meshJobs(meshJobs), _meshJobsMtx(meshJobsMtx)
{ 
	AddEventHandler(EV::def_OP_Push, this, &Driver::Event_Push);
	AddEventHandler(EV::def_SelectionChanged, this, &Driver::Event_SelectionChanged);
	AddEventHandler(EV::def_CameraChanged, this, &Driver::Event_CameraChanged);
	AddEventHandler(EV::def_Mouse, this, &Driver::Event_Mouse);
}

Operator* Driver::GetActiveOperator() {
	SYS::ScopedLock  lock(_activeOpsMtx);
    return ActiveOperator();
}

DWORD Driver::Thread_Func() {
    while(true) { 
        HandleEvents(); 
    }
}

} // namespace OP