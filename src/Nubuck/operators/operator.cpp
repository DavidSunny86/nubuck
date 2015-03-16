#include <QWidget>

#include <Nubuck\events\core_events.h>
#include <Nubuck\animation\animator.h>
#include <Nubuck\operators\operator.h>
#include <world\world_events.h>
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

Operator::Operator() : _panel(NULL) {
    AddEventHandler(ev_mouse, this, &Operator::OnMouse);
    AddEventHandler(ev_key, this, &Operator::OnKey);
    AddEventHandler(ev_w_meshChanged, this, &Operator::OnMeshChanged);
}

NUBUCK_API void WaitForAnimations() {
    g_operators.GetDriver().Wait(A::g_animator.GetConditionVariable(), IsAnimatorIdle);
}

} // namespace OP