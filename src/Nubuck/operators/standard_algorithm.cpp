#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>

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
};

void StandardAlgorithmPanel::OnStep() {
	OP::g_operators.InvokeAction(EV::def_ALG_Step.Create(EV::Params_ALG_Step()));
}

void StandardAlgorithmPanel::OnNext() {
    OP::g_operators.InvokeAction(EV::def_ALG_Next.Create(EV::Params_ALG_Next()));
}

StandardAlgorithmPanel::StandardAlgorithmPanel(QWidget* parent) : OperatorPanel(parent) {
    QPushButton* btnStep = new QPushButton("Step");
    connect(btnStep, SIGNAL(clicked()), this, SLOT(OnStep()));

    QPushButton* btnNext = new QPushButton("Next");
    connect(btnNext, SIGNAL(clicked()), this, SLOT(OnNext()));
    
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(btnStep);
    layout->addWidget(btnNext);
    setLayout(layout);
}

Phase::StepRet::Enum Phase::Step() {
	return StepRet::DONE;
}

GEN::Pointer<Phase> Phase::NextPhase() {
	return GEN::MakePtr(new IdlePhase());
}

bool Phase::IsWall() const { return true; }

void StandardAlgorithm::SetPhase(const GEN::Pointer<Phase>& phase) {
	if(_phase.IsValid()) {
    }
    _phase = phase;
	COM_assert(_phase.IsValid());
    _phase->Enter();
}

void StandardAlgorithm::Event_Step(const EV::Event& event) {
	if(Phase::StepRet::DONE == _phase->Step()) 
		SetPhase(_phase->NextPhase());
}

void StandardAlgorithm::Event_Next(const EV::Event& event) {
    bool done = false;
    while(!done)
    {
        while(Phase::StepRet::DONE != _phase->Step());
        done = _phase->IsWall();
        SetPhase(_phase->NextPhase());
    }
}

StandardAlgorithm::StandardAlgorithm() { 
	AddEventHandler(EV::def_ALG_Step, this, &StandardAlgorithm::Event_Step);
    AddEventHandler(EV::def_ALG_Next, this, &StandardAlgorithm::Event_Next);
}

void StandardAlgorithm::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;
    QAction* action = nb.ui->GetAlgorithmMenu()->addAction(GetName());
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void StandardAlgorithm::Invoke() {
    _nb.ui->SetOperatorName(GetName());

	SetPhase(GEN::Pointer<Phase>(Init(_nb)));
}

} // namespace ALG
} // namespace OP