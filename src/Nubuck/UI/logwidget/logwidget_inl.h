#pragma once

#include <stdarg.h>
#include <stdio.h>
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

    inline void LogWidget::printf(const char* format, ...) {
        static char buffer[1024];
        if(_enabled) {
            memset(buffer, 0, sizeof(buffer));
            va_list args;
            va_start(args, format);
            vsprintf(buffer, format, args);
            va_end(args);

            /*
            QString str(buffer);
            _ui.textBrowser->insertPlainText(str);
            _ui.textBrowser->ensureCursorVisible();
            */
            ::printf("%s", buffer);
        }
    }

} // namespace UI