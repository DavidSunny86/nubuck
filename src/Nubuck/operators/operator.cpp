#include <Nubuck\animation\animator.h>
#include <Nubuck\operators\operator.h>
#include "operator_driver.h"
#include "operators.h"

namespace {

bool IsAnimatorIdle() { return A::g_animator.IsIdle(); }

} // unnamed namespace

namespace OP {

void Operator::WaitForAnimations() {
    g_operators.GetDriver().Wait(A::g_animator.GetConditionVariable(), IsAnimatorIdle);
}

} // namespace OP