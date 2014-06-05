#include <Nubuck\operators\standard_algorithm.h>
#include "flipclip.h"

NUBUCK_OPERATOR OP::OperatorPanel* CreateOperatorPanel() {
    return new FlipClipPanel;
}

NUBUCK_OPERATOR OP::Operator* CreateOperator() {
    return new FlipClip;
}