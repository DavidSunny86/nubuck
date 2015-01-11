#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\editmode\editmode.h>

namespace W {

void EditMode::_NotifyObservers() {
    EV::Arg<int> event(_mode);
    for(unsigned i = 0; i < _obs.size(); ++i) _obs[i]->Send(ev_w_editModeChanged.Tag(event));
}

void EditMode::_SetMode(Enum mode) {
    _mode = mode;
}

EditMode::EditMode() : _mode(editMode_t::DEFAULT) { }

void EditMode::AddObserver(EV::EventHandler<>* const obs) {
    SYS::ScopedLock lock(_mtx);
    _obs.push_back(obs);
}

EditMode::Enum EditMode::GetMode() const { return _mode; }

EditMode::Enum EditMode::GetNextMode() const {
    return Enum((_mode + 1) % editMode_t::NUM_MODES);
}

void EditMode::SetMode(Enum mode) {
    SYS::ScopedLock lock(_mtx);
    _SetMode(mode);
    _NotifyObservers();
}

void EditMode::CycleModes() {
    SYS::ScopedLock lock(_mtx);
    _SetMode(GetNextMode());
    _NotifyObservers();
}

} // namespace W