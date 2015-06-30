#include <operators\operator_events.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <UI\window_events.h>
#include "operators.h"
#include "operator_driver.h"

namespace OP {

// IMPORTANT: must be called for every event
inline void SignalCompletion() {
    g_operators.Send(ev_op_actionFinished.Tag());
}

static int DispatchSingleEvent(Operator* op, const EV::Event& event) {
    COM_assert(event.ret && 0 == *event.ret);
    op->Send(event);
    const int numDispatched = op->HandleEvents();
    COM_assert(1 == numDispatched);
    return *event.ret;
}

void Driver::SetOperator(Operator* op, bool force, const char* args) {
    if(_activeOp == op) return;

    bool invoke = true;

    if(!force && _activeOp && !_activeOp->IsDone()) {
        int retval = 0;
        EV::Event event;
        event.SetReturnPointer(&retval);
        g_operators.SendAndWait(ev_op_showConfirmationDialog.Tag(event));
        if(!retval) invoke = false; // confimration dialog cancelled, abort
    }

	if(invoke) {
        op->SetArgumentData(args);
        if(op->Invoke()) {
            if(_activeOp) _activeOp->Finish();
            _activeOp = op;

            RebuildMeshes();

            SetOperatorEvent event(op, force);
            g_operators.SendAndWait(ev_op_setOperator.Tag(event));
        }
    }

    // TODO hacky hack hack
    // this forces the outliner widget to update its transformation vectors, which
    // is necessary when the user changed them and the invokation of op_set_transform
    // is declined
    W::Entity* ent = W::world.FirstEntity();
    while(ent) {
        if(W::EntityType::ENT_GEOMETRY == ent->GetType()) {
            W::ENT_Geometry* geom = static_cast<W::ENT_Geometry*>(ent);
            geom->SetPosition(ent->GetPosition());
        }
        ent = W::world.NextEntity(ent);
    }
}

void Driver::RebuildMeshes() {
    // let non-active operators react to modified meshes
    EV::Event event = ev_w_meshChanged.Tag();
    int accepted = 0;
    event.SetReturnPointer(&accepted);
    if(_activeOp != _defaultOp) {
        DispatchSingleEvent(_defaultOp, event);
    }

    W::world.SendAndWait(ev_w_rebuildAll.Tag());
}

void Driver::Event_SetOperator(const SetOperatorEvent& event) {
    SetOperator(event.m_op, event.m_force, event.m_args);
    SignalCompletion();
}

void Driver::Event_Fallthrough(const EV::Event& event) {
    int accepted = 0;
    event.SetReturnPointer(&accepted);
	if(!(_activeOp && DispatchSingleEvent(_activeOp, event))) {
        if(_activeOp != _defaultOp) {
            GEN::Pointer<EV::Event> ftevent(event.Clone());
            ftevent->SetFallthrough(true);
            if(DispatchSingleEvent(_defaultOp, *ftevent)) {
                // default operator becomes active, implicit rebuild
                SetOperator(_defaultOp, false, NULL);
                // next, we have to resend the event without fallthrough flag
                accepted = 0;
                DispatchSingleEvent(_activeOp, event);
            }
        }
    }
    if(!accepted) {
        // forward event
        W::world.Send(event);
    }
    RebuildMeshes();
    SignalCompletion();
}

void Driver::Event_RebuildAll(const EV::Event& event) {
    W::world.SendAndWait(ev_w_rebuildAll.Tag());
    SignalCompletion();
}

void Driver::Event_Default(const EV::Event& event, const char* className) {
    Event_Fallthrough(event);
}

Driver::Driver(Operator* defaultOp)
    : _isBlocked(false)
    , _defaultOp(defaultOp)
    , _activeOp(0)
{
    assert(_defaultOp);

	AddEventHandler(ev_op_setOperator, this, &Driver::Event_SetOperator);
    AddEventHandler(ev_w_rebuildAll, this, &Driver::Event_RebuildAll);
}

bool Driver::IsBlocked() const {
    return _isBlocked;
}

void Driver::Wait(SYS::ConditionVariable& cvar, bool (*testFunc)()) {
    SYS::SpinLock mtx;
    mtx.Lock();
    _isBlocked = true;
    while(!testFunc()) {
        cvar.Wait(mtx);
    }
    _isBlocked = false;
    mtx.Unlock();
}

DWORD Driver::Thread_Func() {
    while(true) {
        HandleEvents();
    }
}

} // namespace OP