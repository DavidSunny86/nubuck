#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\editmode\editmode.h>

namespace W {

void EditMode::_NotifyObservers() {
    EV::Params_EditModeChanged args = { _mode };
    EV::Event event = EV::def_EditModeChanged.Create(args);
    for(unsigned i = 0; i < _obs.size(); ++i) _obs[i]->Send(event);
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

void EditMode::SetMode(Enum mode) {
    SYS::ScopedLock lock(_mtx);
    _SetMode(mode);
    _NotifyObservers();
}

void EditMode::CycleModes() {
    SYS::ScopedLock lock(_mtx);
    Enum nextMode = Enum((_mode + 1) % editMode_t::NUM_MODES);
    _SetMode(nextMode);
    _NotifyObservers();
}

} // namespace W