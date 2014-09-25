#pragma once

#ifdef NUBUCK_BUILD_DESIGNER_PLUGIN
#include <QDesignerExportWidget>
#else
#define QDESIGNER_WIDGET_EXPORT /* ... */
#endif

#include <QFrame>
#include <LEDA\numbers\rational.h>

class NBW_SpinBoxControls;

// minor nuisance: you have to copy leda.dll into qt bin directory,
// next to designer.exe

// derive custom widgets from QFrame to make the stylesheet work.
// method proposed in http://stackoverflow.com/questions/7276330/qt-stylesheet-for-custom-widget
// doesn't work when custom widget defines Q_OBJECT
class QDESIGNER_WIDGET_EXPORT NBW_SpinBox : public QFrame {
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)
    // don't expose Q_PROPERTY for min,max,value because
    // we can't use type leda::rational in designer
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
    // stack child widgets for nice progessbar. the stylesheet uses rounded
    // borders that make it hard to draw exactly in the content rect in paintEvent()
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

    QSize sizeHint() const override;

    const QString& text() const;

    leda::rational minimum() const;
    leda::rational maximum() const;
    leda::rational value() const;

    void showProgressBar(bool show);
    void setTypeMask(int mask);

    void setText(const QString& text);

    void setMinimum(const leda::rational val);
    void setMaximum(const leda::rational val);
    void setSingleStep(const leda::rational val);
    void setValue(const leda::rational val);
};