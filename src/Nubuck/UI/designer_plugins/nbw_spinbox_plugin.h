#include <QDesignerCustomWidgetInterface>

class NBW_SpinBoxPlugin : public QObject, public QDesignerCustomWidgetInterface {
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
private:
    bool _isInitialized;
public:
    NBW_SpinBoxPlugin(QObject* parent = 0);

    bool        isContainer() const;
    bool        isInitialized() const;
    QIcon       icon() const;
    QString     group() const;
    QString     includeFile() const;
    QString     name() const;
    QString     toolTip() const;
    QString     whatsThis() const;
    QWidget*    createWidget(QWidget* parent);
    void        initialize(QDesignerFormEditorInterface* core);
};
