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
		return StepRet::CONTINUE;
	}
};

void StandardAlgorithmPanel::OnStep() {
	OP::g_operators.InvokeAction(EV::def_ALG_Step.Create(EV::Params_ALG_Step()));
}

StandardAlgorithmPanel::StandardAlgorithmPanel(QWidget* parent) : OperatorPanel(parent) {
    QPushButton* btnStep = new QPushButton("Step");
    connect(btnStep, SIGNAL(clicked()), this, SLOT(OnStep()));
    
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(btnStep);
    setLayout(layout);
}

Phase::StepRet::Enum Phase::Step() {
	return StepRet::DONE;
}

Phase* Phase::NextPhase() {
    return new IdlePhase();
}

void StandardAlgorithm::SetPhase(const GEN::Pointer<Phase>& phase) {
	if(_phase.IsValid()) {
    }
    _phase = phase;
    _phase->Enter();
}

void StandardAlgorithm::Event_Step(const EV::Event& event) {
	if(Phase::StepRet::DONE == _phase->Step()) 
		SetPhase(GEN::Pointer<Phase>(_phase->NextPhase()));
}

StandardAlgorithm::StandardAlgorithm() { 
	AddEventHandler(EV::def_ALG_Step, this, &StandardAlgorithm::Event_Step);
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