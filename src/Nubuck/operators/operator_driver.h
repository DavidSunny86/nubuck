#pragma once

#include <system\thread\thread.h>
#include <events\events.h>

namespace OP {

class Operator;

class Driver : public SYS::Thread, public EV::EventHandler<EV::EventHandlerPolicies::Blocking> {
private:
    DECL_HANDLE_EVENTS(Driver)

    std::vector<Operator*>& _activeOps;
    SYS::SpinLock&          _activeOpsMtx;

    void Event_SetOperator(const EV::Event& event);
protected:
    void Event_Default(const EV::Event& event, const char* className) override;
public:
    Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx);

    DWORD Thread_Func(void) override;
};

} // namespace OP