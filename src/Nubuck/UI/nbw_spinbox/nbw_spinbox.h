#pragma once

#include <QFrame>
#include <LEDA\numbers\rational.h>

class NBW_SpinBoxControls;

class NBW_SpinBox : public QFrame {
    Q_OBJECT
public:
    struct TypeFlags {
        enum {
            INTEGER     = (1 << 0),
            DOUBLE      = (1 << 1),
            RATIONAL    = (1 << 2),

            ALL         = INTEGER | DOUBLE | RATIONAL
        };
    };
private:
    QFrame*                 _progress;
    NBW_SpinBoxControls*    _controls;

    bool                    _showProgress;

    void ResizeProgressBar();

    static QSize s_preferredSize;
private slots:
    void OnBeginDragging();
    void OnEndDragging();
    void OnValueChanged();
protected:
    void resizeEvent(QResizeEvent* event) override;
signals:
    void SigValueChanged(leda::rational val);
public:
    NBW_SpinBox(QWidget* parent = 0);

    void ShowProgressBar(bool show);

    QSize sizeHint() const override;

    leda::rational minimum() const;
    leda::rational maximum() const;

    void setTypeMask(int mask);

    void setMinimum(const leda::rational val);
    void setMaximum(const leda::rational val);
    void setSingleStep(const leda::rational val);
    void setValue(const leda::rational val);
};