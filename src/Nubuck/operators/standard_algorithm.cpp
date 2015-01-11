#include <iostream>

#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\events\core_events.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\operators\standard_algorithm.h>
#include <operators\operators.h>

EV::ConcreteEventDef<EV::Event> ev_step;
EV::ConcreteEventDef<EV::Event> ev_next;
EV::ConcreteEventDef<EV::Event> ev_run;

namespace OP {
namespace ALG {

struct IdlePhase : Phase {
    void Enter() override {
		std::cout << "entering idle phase" << std::endl;
	}

    StepRet::Enum Step() override {
		return StepRet::DONE;
	}

    bool IsWall() const { return true; }
};

void StandardAlgorithmPanel::OnStep() {
    OP::SendToOperator(ev_step.Tag());
}

void StandardAlgorithmPanel::OnNext() {
    OP::SendToOperator(ev_next.Tag());
}

void StandardAlgorithmPanel::OnRun() {
    OP::SendToOperator(ev_run.Tag());
}

StandardAlgorithmPanel::StandardAlgorithmPanel() {
    QPushButton* btnStep = new QPushButton("Step");
    QObject::connect(btnStep, SIGNAL(clicked()), this, SLOT(OnStep()));

    QPushButton* btnNext = new QPushButton("Next");
    QObject::connect(btnNext, SIGNAL(clicked()), this, SLOT(OnNext()));

    QPushButton* btnRun = new QPushButton("Run");
    QObject::connect(btnRun, SIGNAL(clicked()), this, SLOT(OnRun()));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(btnStep);
    layout->addWidget(btnNext);
    layout->addSpacing(20);
    layout->addWidget(btnRun);
    layout->addSpacing(20);
    GetWidget()->setLayout(layout);
}

Phase::StepRet::Enum Phase::Step() {
	return StepRet::DONE;
}

GEN::Pointer<Phase> Phase::NextPhase() {
	return GEN::MakePtr(new IdlePhase());
}

bool Phase::IsWall() const { return false; }

void StandardAlgorithm::SetPhase(const GEN::Pointer<Phase>& phase) {
	if(_phase.IsValid()) {
        _phase->Leave();
    }
    _phase = phase;
	COM_assert(_phase.IsValid());
    _phase->SetRunConf(&_runConf);
    _phase->Enter();
}

void StandardAlgorithm::Event_Step(const EV::Event& event) {
    _runConf.mode = Phase::RunMode::STEP;
	if(Phase::StepRet::DONE == _phase->Step())
		SetPhase(_phase->NextPhase());
}

void StandardAlgorithm::Event_Next(const EV::Event& event) {
    _runConf.mode = Phase::RunMode::NEXT;
    while(Phase::StepRet::DONE != _phase->Step());
    SetPhase(_phase->NextPhase());
}

void StandardAlgorithm::Event_Run(const EV::Event& event) {
    _runConf.mode = Phase::RunMode::RUN;
    bool done = false;
    while(!done) {
        while(Phase::StepRet::DONE != _phase->Step());
        SetPhase(_phase->NextPhase());
        done = _phase->IsWall();
    }
}

StandardAlgorithm::StandardAlgorithm() {
	AddEventHandler(ev_step, this, &StandardAlgorithm::Event_Step);
    AddEventHandler(ev_next, this, &StandardAlgorithm::Event_Next);
    AddEventHandler(ev_run, this, &StandardAlgorithm::Event_Run);
}

void StandardAlgorithm::Register(Invoker& invoker) {
    QAction* action = NB::AlgorithmMenu()->addAction(GetName());
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool StandardAlgorithm::Invoke() {
    NB::SetOperatorName(GetName());

    Phase* phase = Init();
    if(phase) {
	    SetPhase(GEN::MakePtr(phase));
        return true;
    }

    return false;
}

void StandardAlgorithm::Finish() {
}

void StandardAlgorithm::OnMouse(const EV::MouseEvent& event) {
    // algorithms accepts all mouse events, so default operator
    // op_translate can't become active
    event.Accept();
}

void StandardAlgorithm::OnKey(const EV::KeyEvent& event) {
    // algorithms accepts all key events, so default operator
    // op_translate can't become active

    // hacky HACK: pass through numpad keys to control camera

    bool accept = true;

    // numpad scancodes of generic usb keyboard
    static const int numpad[] = {
        82,
        79, 80, 81,
        75, 76, 77,
        71, 72, 73
    };
    for(int i = 0; i < 10; ++i) {
        if(numpad[i] == event.nativeScanCode) {
            accept = false;
        }
    }

    if(accept) event.Accept();
}

} // namespace ALG
} // namespace OP