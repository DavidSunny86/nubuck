#include <nubuck_private.h>
#include "operators.h"

namespace OP {

Operators g_operators;

unsigned Operators::Register(Operator* op) {
    unsigned id = _ops.size();

    Invoker* invoker = new Invoker(id);
    QObject::connect(invoker, SIGNAL(SigInvokeOperator(unsigned)), this, SLOT(OnInvokeOperator(unsigned)));

    op->Register(nubuck, *invoker);

    OperatorDesc desc;
    desc.id = id;
    desc.invoker = invoker;
    desc.op = op;

    _ops.push_back(desc);

    return id;
}

} // namespace OP