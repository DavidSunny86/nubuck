#pragma once

#include <vector>

#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\events\events.h>

namespace W {

class EditMode {
public:
    enum Enum {
        OBJECTS     = 0,
        VERTICES,

        NUM_MODES,
        DEFAULT     = OBJECTS
    };
private:
    SYS::SpinLock                       _mtx;
    Enum                                _mode;
    std::vector<EV::EventHandler<>*>    _obs;

    void _NotifyObservers();
    void _SetMode(Enum mode);
public:
    EditMode();

    void AddObserver(EV::EventHandler<>* const obs);

    Enum GetMode() const;
    void SetMode(Enum mode);
    void CycleModes();
};

} // namespace W