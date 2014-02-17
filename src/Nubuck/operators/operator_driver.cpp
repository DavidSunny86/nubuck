#include <Nubuck\operators\operator.h>
#include <system\locks\scoped_lock.h>
#include "operator_events.h"
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

void Driver::Event_Default(const EV::Event& event, const char* className) {
    SYS::ScopedLock lock(_activeOpsMtx);
    Operator* op = ActiveOperator();
    if(op) {
        op->Send(event);
        op->HandleEvents();
	}
}

Driver::Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx) : _activeOps(activeOps), _activeOpsMtx(activeOpsMtx) { 
	AddEventHandler(EV::def_OP_Push, this, &Driver::Event_Push);
}

DWORD Driver::Thread_Func() {
    while(true) { 
        printf("Driver: entering loop!!!\n");
        HandleEvents(); 
    }
}

} // namespace OP