#include <QVBoxLayout>
#include <operators\operators.h>
#include "operatorpanel.h"

namespace UI {

OperatorPanel::OperatorPanel(QWidget* parent) : QDockWidget("Operator", parent) {
    _header = new QWidget;
    _headerUi.setupUi(_header);
    _scrollArea = new QScrollArea();
    QDockWidget::setWidget(_scrollArea);
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
    _scrollArea->setWidget(dummy);
    _scrollArea->setWidgetResizable(true);
}

} // namespace UI