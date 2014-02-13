#include <QVBoxLayout>
#include <operators\operators.h>
#include "operatorpanel.h"

namespace UI {

OperatorPanel::OperatorPanel(QWidget* parent) : QDockWidget("Operator", parent) {
    _header = new QWidget;
    _headerUi.setupUi(_header);
    setWidget(NULL);
}

void OperatorPanel::SetOperatorName(const QString& name) {
    _headerUi.lblName->setText(name);
}

void OperatorPanel::setWidget(QWidget* widget) {
    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_header);
    layout->addWidget(widget);
    layout->addStretch();
    QWidget* dummy = new QWidget;
    dummy->setLayout(layout);
    QDockWidget::setWidget(dummy);
}

} // namespace UI