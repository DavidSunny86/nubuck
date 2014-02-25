#include <QPushButton>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>

#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\operators\standard_algorithm.h>

namespace OP {
namespace ALG {

void StandardAlgorithmPanel::OnStep() {
}

StandardAlgorithmPanel::StandardAlgorithmPanel(QWidget* parent) : OperatorPanel(parent) {
    QPushButton* btnStep = new QPushButton("Step");
    connect(btnStep, SIGNAL(clicked()), this, SLOT(OnStep()));
    
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(btnStep);
    setLayout(layout);
}

void StandardAlgorithm::SetPhase(Phase* phase) {
    if(_phase) {
    }
    _phase = phase;
    _phase->Enter();
}

StandardAlgorithm::StandardAlgorithm() : _phase(NULL) { }

void StandardAlgorithm::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;
    QAction* action = nb.ui->GetAlgorithmMenu()->addAction(GetName());
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void StandardAlgorithm::Invoke() {
    _nb.ui->SetOperatorName(GetName());

    SetPhase(Init(_nb));
}

} // namespace ALG
} // namespace OP