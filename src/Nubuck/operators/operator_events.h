#pragma once

#include <Nubuck\events\events.h>

namespace OP {

class Operator;

} // namespace OP

extern EV::ConcreteEventDef<EV::Event>                  ev_op_actionFinished;
extern EV::ConcreteEventDef<EV::Arg<OP::Operator*> >    ev_op_setOperator;