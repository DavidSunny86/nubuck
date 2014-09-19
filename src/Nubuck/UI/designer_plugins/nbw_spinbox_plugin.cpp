#include <QtPlugin>
#include <UI\nbw_spinbox\nbw_spinbox.h>
#include "nbw_spinbox_plugin.h"

NBW_SpinBoxPlugin::NBW_SpinBoxPlugin(QObject* parent)
    : QObject(parent)
    , _isInitialized(false)
{
}

bool NBW_SpinBoxPlugin::isContainer() const {
    return false;
}

bool NBW_SpinBoxPlugin::isInitialized() const {
    return _isInitialized;
}

QIcon NBW_SpinBoxPlugin::icon() const {
    return QIcon();
}

QString NBW_SpinBoxPlugin::group() const {
    return "Nubuck";
}

QString NBW_SpinBoxPlugin::includeFile() const {
    return "UI/nbw_spinbox/nbw_spinbox.h";
}

QString NBW_SpinBoxPlugin::name() const {
    return "NBW_SpinBox";
}

QString NBW_SpinBoxPlugin::toolTip() const {
    return "";
}

QString NBW_SpinBoxPlugin::whatsThis() const {
    return "";
}

QWidget* NBW_SpinBoxPlugin::createWidget(QWidget* parent) {
    return new NBW_SpinBox(parent);
}

void NBW_SpinBoxPlugin::initialize(QDesignerFormEditorInterface*) {
    if(_isInitialized) return;

    _isInitialized = true;
}

Q_EXPORT_PLUGIN2(nubuck_widgets, NBW_SpinBoxPlugin)
