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

    inline void LogWidget::Flush() {
        if(!_buffer.empty()) {
            _ui.textBrowser->setTextColor(QColor(255, 255, 255));
            _ui.textBrowser->setFontPointSize(12);

            _bufferMtx.Lock();

            for(unsigned i = 0; i < _buffer.size(); ++i) {
                QString str = QString::fromStdString(_buffer[i]);
                _ui.textBrowser->insertPlainText(str);
                _ui.textBrowser->ensureCursorVisible();
            }

            _buffer.clear();

            _bufferMtx.Unlock();
        }
    }

    inline void LogWidget::sys_printf(const char* format, ...) {
        static char buffer[1024];
        if(_enabled) {
            memset(buffer, 0, sizeof(buffer));
            va_list args;
            va_start(args, format);
            vsprintf(buffer, format, args);
            va_end(args);

            _ui.textBrowser->setTextColor(QColor(0, 0, 0));
            _ui.textBrowser->setFontPointSize(10);

            _ui.textBrowser->insertPlainText(QString::fromStdString(buffer));
            _ui.textBrowser->ensureCursorVisible();
        }
    }

    inline void LogWidget::printf(const char* format, ...) {
        static char buffer[1024];
        if(_enabled) {
            memset(buffer, 0, sizeof(buffer));
            va_list args;
            va_start(args, format);
            vsprintf(buffer, format, args);
            va_end(args);

            if(g_ui.UI_ThreadID() != SYS::Thread::CallerID())
                _bufferMtx.Lock();

            _buffer.push_back(std::string(buffer));

            if(g_ui.UI_ThreadID() != SYS::Thread::CallerID())
                _bufferMtx.Unlock();
        }
    }

} // namespace UI