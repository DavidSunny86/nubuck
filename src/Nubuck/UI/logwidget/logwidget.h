#pragma once

#include <string>
#include <vector>

#include <QWidget>
#include <Nubuck\nubuck.h>
#include <Nubuck\system\locks\spinlock.h>
#include "ui_logwidget.h"

namespace UI {

    class LogWidget : public QWidget {
        Q_OBJECT
    private:
        Ui::LogWidget   _ui;
        bool            _enabled;

        std::vector<std::string>    _buffer;
        SYS::SpinLock               _bufferMtx;

        LogWidget(QWidget* parent = NULL);
    private slots:
        void OnSetEnabled(bool enable);
    public:
        static LogWidget* Instance(void);

        bool IsEnabled(void) const;

        void Enable(void);
        void Disable(void);

        void Flush();

        void sys_printf(const char* format, ...);

        void printf(const char* format, ...);
    };

} // namespace UI

#include "logwidget_inl.h"