#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <system\thread\thread.h>
#include <UI\userinterface.h>
#include "logwidget.h"

namespace UI {

    inline LogWidget* LogWidget::Instance(void) {
        static LogWidget* instance = NULL;
        if(!instance) instance = new LogWidget();
        return instance;
    }

    inline LogWidget& logWidget(void) {
        return *LogWidget::Instance();
    }

    inline bool LogWidget::IsEnabled(void) const {
        return _enabled;
    }

    inline void LogWidget::Enable(void) { _enabled = true; }
    inline void LogWidget::Disable(void) { _enabled = false; }

} // namespace UI