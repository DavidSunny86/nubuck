#pragma once

#include <system\thread\thread.h>
#include <Nubuck\events\events.h>
#include <Nubuck\events\core_events.h>
#include <renderer\renderer.h>

struct SetOperatorEvent;

namespace OP {

class Operator;

class Driver : public SYS::Thread, public EV::EventHandler<EV::EventHandlerPolicies::Blocking> {
private:
    DECL_HANDLE_EVENTS(Driver)

    bool        _isBlocked;
    Operator*   _defaultOp;
    Operator*   _activeOp;

    void SetOperator(Operator* op, bool force, const char* args);

    void RebuildMeshes();

    void Event_SetOperator(const SetOperatorEvent& event);
    void Event_RebuildAll(const EV::Event& event);

    void Event_Fallthrough(const EV::Event& event);
protected:
    void Event_Default(const EV::Event& event, const char* className) override;
public:
    Driver(Operator* defaultOp);

    unsigned GetEventQueueSize() const { return EV::EventHandler<EV::EventHandlerPolicies::Blocking>::GetEventQueueSize(); }

    bool IsBlocked() const;

    void Wait(SYS::ConditionVariable& cvar, bool (*testFunc)());

    DWORD Thread_Func(void) override;
};

} // namespace OP