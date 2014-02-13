#include <Nubuck\operators\operator.h>
#include <system\locks\scoped_lock.h>
#include "operator_events.h"
#include "operator_driver.h"

namespace OP {

void Driver::Event_SetOperator(const EV::Event& event) {
    const EV::Params_OP_Driver_SetOperator& params = EV::def_OP_Driver_SetOperator.GetArgs(event);
    printf("okay, changing operator!\n");
    event.Accept();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    SYS::ScopedLock lock(_activeOpsMtx);
    printf("OP::Driver: default event. forwarding\n");
    _activeOps.back()->Send(event);
    _activeOps.back()->HandleEvents();
}

Driver::Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx) : _activeOps(activeOps), _activeOpsMtx(activeOpsMtx) { }

DWORD Driver::Thread_Func() {
    while(true) { 
        printf("Driver: entering loop!!!\n");
        HandleEvents(); 
    }
}

} // namespace OP