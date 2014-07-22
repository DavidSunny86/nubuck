#pragma once

#include <Nubuck\events\events.h>


namespace OP {

class Operator;

namespace ED {

BEGIN_EVENT_DEF_CS(ActionFinished)
END_EVENT_DEF_CS

BEGIN_EVENT_DEF_CS(Push)
    Operator* op;
END_EVENT_DEF_CS

BEGIN_EVENT_DEF_CS(Pop)
    unsigned count;
END_EVENT_DEF_CS

} // namespace ED

} // namespace OP
