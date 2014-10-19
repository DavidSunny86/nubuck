#include <QVBoxLayout>
#include <QMouseEvent>
#include <QToolButton>
#include <QMessageBox>
#include <operators\operator_events.h>
#include <operators\operators.h>
#include "operatorpanel.h"

namespace UI {

struct PopOperatorButton : public QToolButton {
protected:
    void mousePressEvent(QMouseEvent* event) {
        if(Qt::LeftButton == event->button() && OP::g_operators.IsDriverIdle()) {
            QMessageBox::StandardButton btn = QMessageBox::question(
                this, "Pop operator", "Do you want to pop this operator?",
                QMessageBox::Yes | QMessageBox::No);
            if(QMessageBox::Yes == btn) {
                OP::ED::Params_SetOperator args;
                args.op = OP::g_operators.GetDefaultOperator();
                OP::g_operators.InvokeAction(OP::ED::def_SetOperator.Create(args));
            }
        }
    }
public:
    PopOperatorButton() {
        setIcon(QIcon(":/ui/Images/pop.svg"));
        setObjectName("selectObject");
    }
};

OperatorPanel::OperatorPanel(QWidget* parent)
    : QDockWidget("Operator", parent)
    , _opWidget(NULL)
{
    _header = new QWidget;
    _headerUi.setupUi(_header);

    PopOperatorButton* btnPopOp = new PopOperatorButton;
    _headerUi.hboxLayout->addWidget(btnPopOp);

    _scrollArea = new QScrollArea();
    QDockWidget::setWidget(_scrollArea);
    setWidget(NULL);
}

void OperatorPanel::SetOperatorName(const QString& name) {
    _headerUi.lblName->setText(name);
}

void OperatorPanel::setWidget(QWidget* widget) {
    if(_opWidget) _opWidget->setParent(0); // prevents opWidget from being deleted

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_header);
    if(widget) {
        layout->addWidget(widget);
    }
    layout->addStretch();
    QWidget* dummy = new QWidget;
    dummy->setLayout(layout);
    _scrollArea->setWidget(dummy);
    _scrollArea->setWidgetResizable(true);

    _opWidget = widget;
}

} // namespace UI