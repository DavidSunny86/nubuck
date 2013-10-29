#pragma once

#include <QWidget>
#include <QColor>

namespace UI {

class ColorButton : public QWidget {
    Q_OBJECT
private:
    QColor _color;
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
signals:
    void SigColorChanged(float r, float g, float b);
public:
    ColorButton(QWidget* parent = NULL);
};

} // namespace UI