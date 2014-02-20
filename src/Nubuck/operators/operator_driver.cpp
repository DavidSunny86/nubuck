#include <Nubuck\operators\operator.h>
#include <system\locks\scoped_lock.h>
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
    printf(">>>>>>>>>> OP::Driver finished\n");
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

void Driver::Event_Mouse(const EV::Event& event) {
    SYS::ScopedLock lock(_activeOpsMtx);
	const EV::Params_Mouse& args = EV::def_Mouse.GetArgs(event);
    M::Vector2 mouseCoords = M::Vector2(args.x, args.y);
	bool shiftKey = args.mods & EV::Params_Mouse::MODIFIER_SHIFT;
    unsigned ret = 0;
    for(int i = _activeOps.size() - 1; !ret && 0 <= i; --i) {
        Operator* op = _activeOps[i];
        bool accepted = false;
		if(EV::Params_Mouse::MOUSE_DOWN == args.type) accepted = op->OnMouseDown(mouseCoords, shiftKey);
		if(EV::Params_Mouse::MOUSE_UP == args.type) accepted = op->OnMouseUp(mouseCoords);
		if(EV::Params_Mouse::MOUSE_MOVE == args.type) accepted = op->OnMouseMove(mouseCoords);
        if(accepted) {
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
        printf("Driver: entering loop!!!\n");
        HandleEvents(); 
    }
}

} // namespace OP