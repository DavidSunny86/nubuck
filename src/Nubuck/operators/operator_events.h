#pragma once

#include <Nubuck\events\events.h>

namespace OP { class Operator; }

BEGIN_EVENT_DEF(OP_ActionFinished)
END_EVENT_DEF

BEGIN_EVENT_DEF(OP_Push)
    OP::Operator* op;
END_EVENT_DEF

BEGIN_EVENT_DEF(OP_Pop)
    unsigned count;
END_EVENT_DEF