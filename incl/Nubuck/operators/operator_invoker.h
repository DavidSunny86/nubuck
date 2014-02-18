#pragma once

#include <QObject>

namespace OP {

class Invoker : public QObject {
    Q_OBJECT
private:
    unsigned _id;
signals:
    void SigInvokeOperator(unsigned id);
public slots:
    void OnInvoke() { emit SigInvokeOperator(_id); }
public:
    explicit Invoker(unsigned id) : _id(id) { }
};

} // namespace OP