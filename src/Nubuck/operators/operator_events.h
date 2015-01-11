#pragma once

#include <Nubuck\events\events.h>

namespace OP {

class Operator;

} // namespace OP

extern EV::ConcreteEventDef<EV::Event>                          ev_op_actionFinished;
extern EV::ConcreteEventDef<EV::Args2<OP::Operator*, bool> >    ev_op_setOperator;
extern EV::ConcreteEventDef<EV::Event>                          ev_op_showConfirmationDialog;