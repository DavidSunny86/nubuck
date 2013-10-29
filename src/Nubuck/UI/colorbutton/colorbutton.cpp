#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

#include "colorbutton.h"

namespace UI {

void ColorButton::mousePressEvent(QMouseEvent* event) {
    if(Qt::LeftButton == event->button()) {
        QColor color = QColorDialog::getColor(_color, this, "Choose Color");
        if(color.isValid()) {
            _color = color;
            emit SigColorChanged(_color.redF(), _color.greenF(), _color.blueF());
        }
    }
}

void ColorButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setBrush(QBrush(_color));
    painter.drawRoundedRect(rect(), 5, 5);
}

ColorButton::ColorButton(QWidget* parent) : QWidget(parent), _color(Qt::black) { }

} // namespace UI