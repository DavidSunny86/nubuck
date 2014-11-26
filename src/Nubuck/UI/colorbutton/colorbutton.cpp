#include <QColorDialog>
#include <QMouseEvent>
#include <QPainter>

#include "colorbutton.h"

namespace UI {

void ColorButton::mousePressEvent(QMouseEvent* event) {
    if(Qt::LeftButton == event->button()) {
        QColor oldColor = _color;
        QColorDialog colorDialog;
        connect(&colorDialog, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(OnColorChanged(const QColor&)));
        if(QDialog::Rejected == colorDialog.exec()) OnColorChanged(oldColor);
    }
}

void ColorButton::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.setBrush(QBrush(_color));
    painter.drawRoundedRect(rect(), 5, 5);
}

void ColorButton::OnColorChanged(const QColor& color) {
    _color = color;
    emit SigColorChanged(color.redF(), color.greenF(), color.blueF());
}

ColorButton::ColorButton(QWidget* parent) : QWidget(parent), _color(Qt::black) { }

void ColorButton::SetColor(float r, float g, float b) {
    _color.setRedF(r);
    _color.setGreenF(g);
    _color.setBlueF(b);
    repaint();
}

const QColor& ColorButton::GetColor() const {
    return _color;
}

} // namespace UI