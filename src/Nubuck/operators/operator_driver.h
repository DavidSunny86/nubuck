#pragma once

#include <system\thread\thread.h>
#include <events\events.h>
#include <renderer\renderer.h>

namespace OP {

class Operator;

class Driver : public SYS::Thread, public EV::EventHandler<EV::EventHandlerPolicies::Blocking> {
private:
    DECL_HANDLE_EVENTS(Driver)

    std::vector<Operator*>& _activeOps;
    SYS::SpinLock&          _activeOpsMtx;

	std::vector<R::MeshJob>&    _meshJobs;
    SYS::SpinLock&              _meshJobsMtx;

    Operator* ActiveOperator();

    void Event_Push(const EV::Event& event);

    void Event_SelectionChanged(const EV::Event& event);
    void Event_CameraChanged(const EV::Event& event);
    void Event_Mouse(const EV::Event& event);
protected:
    void Event_Default(const EV::Event& event, const char* className) override;
public:
    Driver(
		std::vector<Operator*>& activeOps, SYS::SpinLock& activeOpsMtx,
        std::vector<R::MeshJob>& meshJobs, SYS::SpinLock& meshJobsMtx
		);

    Operator* GetActiveOperator();

    DWORD Thread_Func(void) override;
};

} // namespace OP