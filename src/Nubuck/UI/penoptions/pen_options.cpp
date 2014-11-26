#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDoubleSpinBox>
#include <QMouseEvent>
#include <QPainter>

#include <UI\colorbutton\colorbutton.h>
#include "pen_options.h"

namespace UI {

class ColorPreset : public QWidget {
    Q_OBJECT
private:
    QColor _color;
protected:
    void mousePressEvent(QMouseEvent* event) {
        if(Qt::LeftButton == event->button()) {
            emit SigColorChanged(_color);
        }
    }

    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setPen(QPen(Qt::NoPen));
        painter.setBrush(QBrush(_color));
        int size = std::min(rect().height(), rect().width()) / 2;
        painter.drawEllipse(rect().center(), size, size);
    }
signals:
    void SigColorChanged(const QColor& color);
public:
    ColorPreset(QWidget* parent, const QColor& color) : QWidget(parent), _color(color) {
    }

    QSize sizeHint() const override {
        return QSize(50, 10);
    }
};

PenOptions::PenOptions(QWidget* parent) : QWidget(parent) {
    _size = new QDoubleSpinBox;
    _size->setMinimum(0.5);
    _size->setMaximum(10.0);
    _size->setValue(2.0);

    _color = new ColorButton;

    QHBoxLayout* lineLayout = new QHBoxLayout;
    QColor presetColors[] = {
        QColor(0, 0, 0),
        QColor(255, 0, 0),
        QColor(0, 255, 0),
        QColor(0, 0, 255)
    };
    const unsigned numPresets = sizeof(presetColors) / sizeof(presetColors[0]);
    for(unsigned i = 0; i < numPresets; ++i) {
        ColorPreset* preset = new ColorPreset(NULL, presetColors[i]);
        connect(preset, SIGNAL(SigColorChanged(const QColor&)), _color, SLOT(OnColorChanged(const QColor&)));
        lineLayout->addWidget(preset);
    }

    QFormLayout* layout = new QFormLayout;
    layout->addRow("size:", _size);
    layout->addRow("color:", _color);
    layout->addRow("presets:", lineLayout);
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

// trigger moc
#include "pen_options.moc"