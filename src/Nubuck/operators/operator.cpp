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

void Operator::SetArgumentData(const char* args) {
    if(args) {
        memcpy(_args, args, GetArgumentDataSize());
    }
}

const char* Operator::GetArgumentData() const {
    return _args;
}

int Operator::GetArgumentDataSize() const {
    return sizeof(char) * ARGS_BUFFER_SIZE;
}

Operator::Operator() : _panel(NULL) {
    memset(_args, 0, GetArgumentDataSize());

    AddEventHandler(ev_mouse, this, &Operator::OnMouse);
    AddEventHandler(ev_key, this, &Operator::OnKey);
    AddEventHandler(ev_op_requestFinish, this, &Operator::OnRequestFinish);
    AddEventHandler(ev_w_meshChanged, this, &Operator::OnMeshChanged);
}

// string gets passed to QKeySequence
std::string Operator::PreferredShortcut() const {
    return "";
}

void Operator::OnRequestFinish(const EV::Event& event) {
    int ret = 0;
    EV::ShowQuestionBox q;
    q.SetReturnPointer(&ret);
    q.caption = "Switch operators.";
    q.message =
        "This operation ends the currently active operator.\n"
        "Do you want to continue?";
    g_operators.SendAndWait(ev_ui_showQuestionBox.Tag(q));
    if(ret) event.Accept();
}

NUBUCK_API void WaitForAnimations() {
    g_operators.GetDriver().Wait(A::g_animator.GetConditionVariable(), IsAnimatorIdle);
}

} // namespace OP