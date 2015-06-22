#pragma once

#include <Nubuck\events\events.h>

namespace OP {

class Operator;

} // namespace OP

struct SetOperatorEvent : public EV::Event {
    EVENT_TYPE(SetOperatorEvent)

    enum { ARGS_BUFFER_SIZE = 128 };

    OP::Operator*   m_op;
    bool            m_force;
    char            m_args[ARGS_BUFFER_SIZE];

    SetOperatorEvent(OP::Operator* op = NULL, bool force = false)
        : m_op(op)
        , m_force(force)
    {
        memset(m_args, 0, sizeof(m_args));
    }

    SetOperatorEvent(const SetOperatorEvent& other)
        : Event(other)
        , m_op(other.m_op)
        , m_force(other.m_force)
    {
        memcpy(m_args, other.m_args, sizeof(m_args));
    }
};

extern EV::ConcreteEventDef<EV::Event>          ev_op_actionFinished;
extern EV::ConcreteEventDef<SetOperatorEvent>   ev_op_setOperator;
extern EV::ConcreteEventDef<EV::Event>          ev_op_showConfirmationDialog;