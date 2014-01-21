#pragma once

#include <QObject>

#include <vector>
#include <UI\operatorpanel\operatorpanel.h>
#include "operator.h"

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

class Operators : public QObject {
    Q_OBJECT
private:
    struct OperatorDesc {
        unsigned    id;
        Operator*   op;
        Invoker*    invoker;
    };

    std::vector<OperatorDesc>   _ops;
    Operator*                   _activeOp;
public slots:
    void OnInvokeOperator(unsigned id) {
        printf("invoking operator with id = %d\n", id);
        if(_activeOp) _activeOp->Finish();
        UI::OperatorPanel::Instance()->Clear();
        _activeOp = _ops[id].op;
        _activeOp->Invoke();
    }
public:
    Operators() : _activeOp(NULL) { }

    void Register(Operator* op);

    Operator* ActiveOperator() { return _activeOp; }
};

extern Operators g_operators;

} // namespace OP