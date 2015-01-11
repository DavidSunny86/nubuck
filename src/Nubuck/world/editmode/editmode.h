#pragma once

#include <vector>

#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\events\events.h>
#include <Nubuck\world\editmode.h>

namespace W {

class EditMode {
private:
    typedef editMode_t::Enum Enum;

    SYS::SpinLock                       _mtx;
    Enum                                _mode;
    std::vector<EV::EventHandler<>*>    _obs;

    void _NotifyObservers();
    void _SetMode(Enum mode);
public:
    EditMode();

    void AddObserver(EV::EventHandler<>* const obs);

    Enum GetMode() const;
    Enum GetNextMode() const;
    void SetMode(Enum mode);
    void CycleModes();
};

} // namespace W