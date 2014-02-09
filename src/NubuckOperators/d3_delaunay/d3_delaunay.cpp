#include <QVBoxLayout>
#include <QPushButton>
#include <QMenu>

#include <Nubuck\operators\operator_invoker.h>
#include "d3_delaunay.h"

namespace OP {

static QString names[] = { "Click Me!", "Me Click!" };

void D3_Delaunay::BuildPanelWidget() {
    _button = new QPushButton(names[0]);

    QVBoxLayout* vboxLayout = new QVBoxLayout;
	vboxLayout->addWidget(_button);
    vboxLayout->addStretch();
    _panel = new QWidget;
    _panel->setLayout(vboxLayout);

    connect(_button, SIGNAL(clicked()), this, SLOT(OnChangeButtonName()));
}

void D3_Delaunay::OnChangeButtonName() {
    _nameIdx = 1 - _nameIdx;
	_button->setText(names[_nameIdx]);
}

void D3_Delaunay::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    printf("ATTENTION PLEASE! Loaded operator D3_Delaunay...\n");

	QAction* action = nb.ui->GetObjectMenu()->addAction("Delaunay 3D");
	connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));

    BuildPanelWidget();
}

void D3_Delaunay::Invoke() {
    printf("ATTENTION PLEASE! Invoked operator D3_Delaunay...\n");
	_nb.ui->SetOperatorPanel(_panel);
}

void D3_Delaunay::Finish() {
}

} // namespace OP

NUBUCK_API OP::Operator* CreateOperator() {
    return new OP::D3_Delaunay();
}