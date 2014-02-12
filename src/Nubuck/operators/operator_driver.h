#pragma once

#include <system\thread\thread.h>
#include <events\events.h>

namespace OP {

class Operator;

class Driver : public SYS::Thread, public EV::EventHandler<Driver, EV::EventHandlerPolicies::Blocking> {
    DECLARE_EVENT_HANDLER(Driver)
private:
    std::vector<Operator*>& _activeOps;
    SYS::ConditionVariable& _activeOpsMtx;

    void Event_SetOperator(const EV::Event& event);
protected:
    void Event_Default(const EV::Event& event);
public:
    Driver(std::vector<Operator*>& activeOps, SYS::ConditionVariable& activeOpsMtx);

    DWORD Thread_Func(void) override;
};

} // namespace OP