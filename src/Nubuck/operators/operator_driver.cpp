#include <operators\operator_events.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
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

void Driver::SetOperator(Operator* op, bool force) {
    if(_activeOp == op) return;

    if(!force && _activeOp && !_activeOp->IsDone()) {
        int retval = 0;
        EV::Event event;
        event.SetReturnPointer(&retval);
        g_operators.SendAndWait(ev_op_showConfirmationDialog.Tag(event));
        if(!retval) return; // confimration dialog cancelled, abort
    }

	if(op->Invoke()) {
        if(_activeOp) _activeOp->Finish();
        _activeOp = op;

        W::world.SendAndWait(ev_w_rebuildAll.Tag());

        EV::Args2<Operator*, bool> event(op, force);
        g_operators.SendAndWait(ev_op_setOperator.Tag(event));
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

void Driver::Event_SetOperator(const EV::Args2<Operator*, bool>& event) {
    SetOperator(event.value0, event.value1);
    SignalCompletion();
}

void Driver::Event_UsrSelectEntity(const EV::Usr_SelectEntity& event) {
    Event_Fallthrough(event);
}

void Driver::Event_UsrChangeEditMode(const EV::Arg<int>& event) {
    Event_Fallthrough(event);
}

void Driver::Event_SelectionChanged(const EV::Event& event) {
    if(_activeOp) _activeOp->OnGeometrySelected();
    _defaultOp->OnGeometrySelected();
    SignalCompletion();
}

void Driver::Event_EditModeChanged(const EV::Arg<int>& event) {
    const W::editMode_t::Enum editMode = W::editMode_t::Enum(event.value);
    if(_activeOp) _activeOp->OnEditModeChanged(editMode);
    if(_activeOp != _defaultOp) _defaultOp->OnEditModeChanged(editMode);
    SignalCompletion();
}

void Driver::Event_Mouse(const EV::MouseEvent& event) {
    Event_Fallthrough(event);
}

void Driver::Event_Key(const EV::KeyEvent& event) {
    Event_Fallthrough(event);
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
                SetOperator(_defaultOp, false);
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
    if(_activeOp) {
        _activeOp->Send(event);
        int numDispatched = _activeOp->HandleEvents();
        COM_assert(1 == numDispatched);

        W::world.SendAndWait(ev_w_rebuildAll.Tag());
	}

    SignalCompletion();
}

Driver::Driver(Operator* defaultOp)
    : _isBlocked(false)
    , _defaultOp(defaultOp)
    , _activeOp(0)
{
    assert(_defaultOp);

	AddEventHandler(ev_op_setOperator, this, &Driver::Event_SetOperator);
    AddEventHandler(ev_usr_selectEntity, this, &Driver::Event_UsrSelectEntity);
    AddEventHandler(ev_usr_changeEditMode, this, &Driver::Event_UsrChangeEditMode);
	AddEventHandler(ev_w_selectionChanged, this, &Driver::Event_SelectionChanged);
    AddEventHandler(ev_w_editModeChanged, this, &Driver::Event_EditModeChanged);
	AddEventHandler(ev_mouse, this, &Driver::Event_Mouse);
    AddEventHandler(ev_key, this, &Driver::Event_Key);
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