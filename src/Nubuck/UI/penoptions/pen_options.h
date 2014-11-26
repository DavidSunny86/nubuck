#pragma once

#include <QWidget>

#include <Nubuck\renderer\color\color.h>

class QDoubleSpinBox;
class ColorButton;

namespace UI {

class PenOptions : public QWidget {
private:
    QDoubleSpinBox* _size;
    ColorButton*    _color;
public:
    PenOptions(QWidget* parent = NULL);

    float       GetSize() const;
    R::Color    GetColor() const;
};

} // namespace UI