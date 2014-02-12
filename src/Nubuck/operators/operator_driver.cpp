#include <Nubuck\operators\operator.h>
#include "operator_events.h"
#include "operator_driver.h"

namespace OP {

BEGIN_EVENT_HANDLER(Driver)
    EVENT_HANDLER(EV::def_OP_Driver_SetOperator, &Driver::Event_SetOperator)
END_EVENT_HANDLER

void Driver::Event_SetOperator(const EV::Event& event) {
    const EV::Params_OP_Driver_SetOperator& params = EV::def_OP_Driver_SetOperator.GetArgs(event);
    printf("okay, changing operator!\n");
    event.Accept();
}

void Driver::Event_Default(const EV::Event& event) {
    printf("OP::Driver: default event. forwarding\n");
    _activeOps.back()->DoAction(event);
}

Driver::Driver(std::vector<Operator*>& activeOps, SYS::ConditionVariable& activeOpsMtx) : _activeOps(activeOps), _activeOpsMtx(activeOpsMtx) { }

DWORD Driver::Thread_Func() {
    while(true) { 
        printf("Driver: entering loop!!!\n");
        HandleEvents(); 
    }
}

} // namespace OP