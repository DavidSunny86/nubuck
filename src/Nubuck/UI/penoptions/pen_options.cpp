#include <QLabel>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <UI\colorbutton\colorbutton.h>
#include "pen_options.h"

namespace UI {

PenOptions::PenOptions(QWidget* parent) : QWidget(parent) {
    _size = new QDoubleSpinBox;
    _size->setMinimum(0.5);
    _size->setMaximum(10.0);
    _size->setValue(2.0);

    _color = new ColorButton;

    QFormLayout* layout = new QFormLayout;
    layout->addRow("size:", _size);
    layout->addRow("color:", _color);
    setLayout(layout);
}

float PenOptions::GetSize() const {
    return static_cast<float>(_size->value());
}

R::Color PenOptions::GetColor() const {
    const QColor& c = _color->GetColor();
    return R::Color(c.redF(), c.greenF(), c.blueF());
}

} // namespace UI