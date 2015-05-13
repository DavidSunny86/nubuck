#pragma once

#include <QObject>
#include <QKeySequence>

namespace OP {

class Invoker : public QObject {
    Q_OBJECT
private:
    unsigned        _id;
    QKeySequence    _shortcut;
signals:
    void SigInvokeOperator(unsigned id);
public slots:
    void OnInvoke() { emit SigInvokeOperator(_id); }
public:
    explicit Invoker(unsigned id, const QKeySequence& shortcut)
        : _id(id)
        , _shortcut(shortcut)
    { }

    QKeySequence GetShortcut() const { return _shortcut; }
};

} // namespace OP