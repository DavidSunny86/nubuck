#pragma once

#ifdef NUBUCK_LIB
#define NUBUCK_API __declspec(dllexport)
#else
#define NUBUCK_API __declspec(dllimport)
#endif

#define NUBUCK_OPERATOR extern "C" __declspec(dllexport)

#define NUBUCK_EXPORT_OPERATOR(op)                              \
    NUBUCK_OPERATOR OP::OperatorPanel* CreateOperator() {       \
        return new op;                                          \
    }

#define NUBUCK_EXPORT_OPERATOR_AND_PANEL(op, pl)                \
    NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {  \
        return new pl;                                          \
    }                                                           \
    NUBUCK_OPERATOR OP::Operator* CreateOperator() {            \
        return new op;                                          \
    }