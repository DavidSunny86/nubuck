#include <QVBoxLayout>
#include <operators\operators.h>
#include "operatorpanel.h"

namespace UI {

OperatorPanel::OperatorPanel(QWidget* parent)
    : QDockWidget("Operator", parent)
    , _opWidget(NULL)
{
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