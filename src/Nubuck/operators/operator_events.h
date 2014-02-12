#pragma once

#include <events\events.h>

namespace OP { class Operator; }

BEGIN_EVENT_DEF(OP_ActionFinished)
END_EVENT_DEF

BEGIN_EVENT_DEF(OP_Driver_SetOperator)
    OP::Operator* op;
END_EVENT_DEF