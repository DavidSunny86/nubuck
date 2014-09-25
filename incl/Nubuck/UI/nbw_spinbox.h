#pragma once

// when specifiying QDESIGNER_WIDGET_EXPORT and NUBUCK_API (or Q_DECL_EXPORT) for
// the same class, no moc file is generated for that class by automoc.

#ifdef NUBUCK_BUILD_DESIGNER_PLUGIN
#include <QDesignerExportWidget>
#define NUBUCK_WIDGET QDESIGNER_WIDGET_EXPORT
#else
#define NUBUCK_WIDGET NUBUCK_API
#endif

#include <QFrame>
#include <LEDA\numbers\rational.h>
#include <Nubuck\nubuck_api.h>

class NBW_SpinBoxControls;

// minor nuisance: you have to copy leda.dll into qt bin directory,
// next to designer.exe

// derive custom widgets from QFrame to make the stylesheet work.
// method proposed in http://stackoverflow.com/questions/7276330/qt-stylesheet-for-custom-widget
// doesn't work when custom widget defines Q_OBJECT
class NUBUCK_WIDGET NBW_SpinBox : public QFrame {
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