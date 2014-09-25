#include <QVBoxLayout>
#include <QSpacerItem>
#include <QLabel>
#include <Nubuck\UI\nbw_spinbox.h>
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

QComboBox* SimplePanel::AddComboBox(const QString& str, const std::vector<QString>& items) {
    int r = _grid->rowCount();
    QComboBox* comboBox = new QComboBox();
    for(unsigned i = 0; i < items.size(); ++i)
        comboBox->addItem(items[i]);
    _grid->addWidget(new QLabel(str), r, 0);
    _grid->addWidget(comboBox, r, 1);
    return comboBox;
}

NBW_SpinBox* SimplePanel::AddSpinBox(const QString& str, int min, int max) {
    int r = _grid->rowCount();
    NBW_SpinBox* spinBox = new NBW_SpinBox();
    spinBox->setTypeMask(NBW_SpinBox::TypeFlags::INTEGER);
    spinBox->setMinimum(min);
    spinBox->setMaximum(max);
    spinBox->setText(str + ": ");
    _grid->addWidget(spinBox, r, 0, 1, 2);
    return spinBox;
}

QPushButton* SimplePanel::AddPushButton(const QString& str) {
    int r = _grid->rowCount();
    QPushButton* button = new QPushButton(str);
    _grid->addWidget(button, r, 0, 1, 2);
    return button;
}

QCheckBox* SimplePanel::AddCheckBox(const QString& str) {
    int r = _grid->rowCount();
    QCheckBox* checkBox = new QCheckBox(str);
    _grid->addWidget(checkBox, r, 0, 1, 2);
    return checkBox;
}

void SimplePanel::AddVerticalSpace(int space) {
    QSpacerItem* spacer = new QSpacerItem(0, space);
    int r = _grid->rowCount();
    _grid->addItem(spacer, r, 0, 1, 2);
}

} // namespace UI