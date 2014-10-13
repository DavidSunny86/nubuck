#pragma once

#include <system\thread\thread.h>
#include <Nubuck\events\events.h>
#include <renderer\renderer.h>

namespace OP {

class Operator;

class Driver : public SYS::Thread, public EV::EventHandler<EV::EventHandlerPolicies::Blocking> {
private:
    DECL_HANDLE_EVENTS(Driver)

    bool _isBlocked;

    std::vector<Operator*>& _activeOps;
    SYS::SpinLock&          _activeOpsMtx;

    Operator* ActiveOperator();

    void Event_Push(const EV::Event& event);

    void Event_SelectionChanged(const EV::Event& event);
    void Event_CameraChanged(const EV::Event& event);
    void Event_EditModeChanged(const EV::Event& event);
    void Event_Mouse(const EV::Event& event);
    void Event_Key(const EV::Event& event);
    void Event_RebuildAll(const EV::Event& event);
protected:
    void Event_Default(const EV::Event& event, const char* className) override;
public:
    Driver(std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx);

    unsigned GetEventQueueSize() const { return EV::EventHandler<EV::EventHandlerPolicies::Blocking>::GetEventQueueSize(); }

    bool IsBlocked() const;

    void Wait(SYS::ConditionVariable& cvar, bool (*testFunc)());

    DWORD Thread_Func(void) override;
};

} // namespace OP