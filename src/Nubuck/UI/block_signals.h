#pragma once

#include <Nubuck\generic\uncopyable.h>
#include <QObject>

namespace UI {

class BlockSignals : private GEN::Uncopyable {
private:
    QObject*    _obj;
    bool        _oldState;
public:
    explicit BlockSignals(QObject* obj) : _obj(obj), _oldState(obj->blockSignals(true)) { }
    ~BlockSignals() { _obj->blockSignals(_oldState); }
};

} // namespace UI
