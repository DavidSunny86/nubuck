#include <QWidget>

#include <Nubuck\animation\animator.h>
#include <Nubuck\operators\operator.h>
#include "operator_driver.h"
#include "operators.h"

namespace {

bool IsAnimatorIdle() { return A::g_animator.IsIdle(); }

} // unnamed namespace

namespace OP {

void OperatorPanel::SetLayout(NB::BoxLayout layout) {
    _widget->setLayout(layout);
}

OperatorPanel::OperatorPanel() : _widget(NULL) {
    _widget = new QWidget;
}

NUBUCK_API void WaitForAnimations() {
    g_operators.GetDriver().Wait(A::g_animator.GetConditionVariable(), IsAnimatorIdle);
}

} // namespace OP