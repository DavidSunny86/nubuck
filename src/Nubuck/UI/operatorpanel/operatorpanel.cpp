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
                EV::Args2<OP::Operator*, bool> event;
                event.value0 = OP::g_operators.GetDefaultOperator();
                event.value1 = true;
                OP::g_operators.InvokeAction(ev_op_setOperator.Tag(event));
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
    : QWidget(parent)
    , _opWidget(NULL)
{
    _header = new QWidget;
    _headerUi.setupUi(_header);

    PopOperatorButton* btnPopOp = new PopOperatorButton;
    _headerUi.hboxLayout->addWidget(btnPopOp);

    setWidget(NULL); // for initial size
}

void OperatorPanel::SetOperatorName(const QString& name) {
    _headerUi.lblName->setText(name);
}

void OperatorPanel::setWidget(QWidget* widget) {
    if(_opWidget) _opWidget->setParent(0); // prevents opWidget from being deleted

    if(layout()) delete layout();

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_header);
    if(widget) {
        layout->addWidget(widget);
    }
    layout->addStretch();
    setLayout(layout);

    _opWidget = widget;
}

} // namespace UI