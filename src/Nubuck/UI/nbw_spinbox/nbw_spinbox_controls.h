#pragma once

#include <QFrame>
#include <QLineEdit>
#include <LEDA\numbers\rational.h>

class NBW_SpinBoxControls : public QFrame {
    Q_OBJECT
private:
    static const float  s_padding;
    static const QSizeF s_arrowSize;

    enum { LEFT_ARROW = 0, RIGHT_ARROW };

    QRectF _arrowRegions[2];
    QRectF _textRegion;

    leda::rational  _rval;
    leda::rational 	_singleStep;
    leda::rational  _min;
    leda::rational  _max;

    int _typeMask;

    void SetValue(const leda::rational rval);

    enum State { STATE_NORMAL, STATE_DRAGGING, STATE_EDIT };

    State _state;

    QPoint          _dragOrigin;
    QPoint          _mousePos;
    leda::rational  _rval0;
    QPoint          _distAccu;

    QLineEdit* _lineEdit;

    void BeginDragging();
    void EndDragging();
private slots:
    void OnEditingFinished();
protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
signals:
    void SigBeginDragging();
    void SigEndDragging();
    void SigValueChanged();
public:
    NBW_SpinBoxControls(QWidget* parent = 0);

    leda::rational GetValue() const;

    leda::rational minimum() const;
    leda::rational maximum() const;

    void setTypeMask(int mask);

    void setMinimum(const leda::rational val);
    void setMaximum(const leda::rational val);
    void setSingleStep(const leda::rational val);
    void setValue(const leda::rational val);
};