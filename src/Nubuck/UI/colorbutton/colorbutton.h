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
private slots:
    void OnColorChanged(const QColor& color);
signals:
    void SigColorChanged(float r, float g, float b);
public:
    ColorButton(QWidget* parent = NULL);

    void            SetColor(float r, float g, float b);
    const QColor&   GetColor() const;
};

} // namespace UI