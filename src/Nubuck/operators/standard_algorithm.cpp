#include <iostream>

#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\operators\standard_algorithm.h>
#include <operators\operators.h>

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
	OP::g_operators.InvokeAction(EV::def_ALG_Step.Create(EV::Params_ALG_Step()));
}

void StandardAlgorithmPanel::OnNext() {
    OP::g_operators.InvokeAction(EV::def_ALG_Next.Create(EV::Params_ALG_Next()));
}

void StandardAlgorithmPanel::OnRun() {
    OP::g_operators.InvokeAction(EV::def_ALG_Run.Create(EV::Params_ALG_Run()));
}

StandardAlgorithmPanel::StandardAlgorithmPanel(QWidget* parent) : OperatorPanel(parent) {
    QPushButton* btnStep = new QPushButton("Step");
    connect(btnStep, SIGNAL(clicked()), this, SLOT(OnStep()));

    QPushButton* btnNext = new QPushButton("Next");
    connect(btnNext, SIGNAL(clicked()), this, SLOT(OnNext()));

    QPushButton* btnRun = new QPushButton("Run");
    connect(btnRun, SIGNAL(clicked()), this, SLOT(OnRun()));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(btnStep);
    layout->addWidget(btnNext);
    layout->addSpacing(20);
    layout->addWidget(btnRun);
    layout->addSpacing(20);
    setLayout(layout);
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
	AddEventHandler(EV::def_ALG_Step, this, &StandardAlgorithm::Event_Step);
    AddEventHandler(EV::def_ALG_Next, this, &StandardAlgorithm::Event_Next);
    AddEventHandler(EV::def_ALG_Run, this, &StandardAlgorithm::Event_Run);
}

void StandardAlgorithm::Register(const Nubuck& nb, Invoker& invoker) {
    QAction* action = nubuck().algorithm_menu()->addAction(GetName());
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool StandardAlgorithm::Invoke() {
    nubuck().set_operator_name(GetName());

    Phase* phase = Init();
    if(phase) {
	    SetPhase(GEN::MakePtr(phase));
        return true;
    }

    return false;
}

} // namespace ALG
} // namespace OP