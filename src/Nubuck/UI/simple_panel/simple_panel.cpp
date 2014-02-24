#include <QVBoxLayout>
#include <QLabel>
#include "simple_panel.h"

namespace UI {

SimplePanel::SimplePanel(QWidget* parent) : OperatorPanel(parent) {
    _grid = new QGridLayout();

    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(_grid);
    vbox->addStretch();
    setLayout(vbox);
}

void SimplePanel::AddLabel(const QString& str) {
    int r = _grid->rowCount();
    _grid->addWidget(new QLabel(str), r, 0, 1, 2);
}

QSpinBox* SimplePanel::AddSpinBox(const QString& str, int min, int max) {
    int r = _grid->rowCount();
    QSpinBox* spinBox = new QSpinBox();
    spinBox->setRange(min, max);
    _grid->addWidget(new QLabel(str), r, 0);
    _grid->addWidget(spinBox, r, 1);
    return spinBox;
}

} // namespace UI