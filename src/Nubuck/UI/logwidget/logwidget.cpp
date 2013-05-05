#include "logwidget.h"

namespace UI {

    void LogWidget::OnSetEnabled(bool enabled) {
        _enabled = enabled;
    }

    LogWidget::LogWidget(QWidget* parent) : QWidget(parent), _enabled(true) {
        _ui.setupUi(this);
    }

} // namespace UI