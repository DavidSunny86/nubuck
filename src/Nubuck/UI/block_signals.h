#pragma once

#include <Nubuck\generic\uncopyable.h>
#include <QObject>

namespace UI {

class BlockSignals : private GEN::Uncopyable {
private:
    QObject*    _obj0;
    QObject*    _obj1;
    QObject*    _obj2;

    bool        _oldState0;
    bool        _oldState1;
    bool        _oldState2;
public:
    explicit BlockSignals(QObject* obj0)
        : _obj0(obj0)
        , _obj1(NULL)
        , _obj2(NULL)
        , _oldState0(obj0->blockSignals(true))
        , _oldState1()
        , _oldState2()
    { }

    BlockSignals(QObject* obj0, QObject* obj1)
        : _obj0(obj0)
        , _obj1(obj1)
        , _obj2(NULL)
        , _oldState0(obj0->blockSignals(true))
        , _oldState1(obj1->blockSignals(true))
        , _oldState2()
    { }

    BlockSignals(QObject* obj0, QObject* obj1, QObject* obj2)
        : _obj0(obj0)
        , _obj1(obj1)
        , _obj2(obj2)
        , _oldState0(obj0->blockSignals(true))
        , _oldState1(obj1->blockSignals(true))
        , _oldState2(obj2->blockSignals(true))
    { }

    ~BlockSignals() {
        if(_obj0) _obj0->blockSignals(_oldState0);
        if(_obj1) _obj1->blockSignals(_oldState1);
        if(_obj2) _obj2->blockSignals(_oldState2);
    }
};

} // namespace UI
