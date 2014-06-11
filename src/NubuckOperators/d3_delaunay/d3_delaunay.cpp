#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>

#include <Nubuck\operators\operator_invoker.h>
#include "d3_delaunay.h"

namespace OP {

void D3_DelaunayPanel::BuildPanelWidget() {
    _button = new QPushButton(_names[0]);

    QVBoxLayout* vboxLayout = new QVBoxLayout;
	vboxLayout->addWidget(_button);
    vboxLayout->addStretch();
    setLayout(vboxLayout);

    connect(_button, SIGNAL(clicked()), this, SLOT(OnChangeButtonName()));
}

void D3_DelaunayPanel::OnChangeButtonName() {
    _nameIdx = 1 - _nameIdx;
	_button->setText(_names[_nameIdx]);

    EV::Params_D3_Delaunay_Action args;
    SendToOperator(EV::def_D3_Delaunay_Action.Create(args));
}

D3_DelaunayPanel::D3_DelaunayPanel(QWidget* parent) : OperatorPanel(parent) {
    _nameIdx = 0;
    _names[0] = "Click Me!";
    _names[1] = "Me Click!";

    BuildPanelWidget();
}

void D3_Delaunay::Event_Action(const EV::Event& event) {
    printf("D3_Delaunay::Event_Action.\n");
}

D3_Delaunay::D3_Delaunay() {
    AddEventHandler(EV::def_D3_Delaunay_Action, this, &D3_Delaunay::Event_Action);
}

void D3_Delaunay::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    printf("ATTENTION PLEASE! Loaded operator D3_Delaunay...\n");

	QAction* action = nb.ui->GetObjectMenu()->addAction("Delaunay 3D");
	QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));

}

bool D3_Delaunay::Invoke() {
    printf("ATTENTION PLEASE! Invoked operator D3_Delaunay...\n");
    return true;
}

void D3_Delaunay::Finish() {
}

} // namespace OP

NUBUCK_API QWidget* CreateOperatorPanel() {
    return new OP::D3_DelaunayPanel();
}

NUBUCK_API OP::Operator* CreateOperator() {
    return new OP::D3_Delaunay();
}