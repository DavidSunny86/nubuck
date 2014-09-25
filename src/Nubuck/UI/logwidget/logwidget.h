#pragma once

#include <string>
#include <vector>

#include <QWidget>
#include <Nubuck\system\locks\spinlock.h>
#include "ui_logwidget.h"

namespace UI {

    class LogWidget : public QWidget {
        Q_OBJECT
    private:
        Ui::LogWidget   _ui;
        bool            _enabled;

        QBrush          _defaultFgBrush;

        std::vector<char>   _buffer;
        char*               _bufferCur;
        SYS::SpinLock       _bufferMtx;

        unsigned            _blockCnt;

        LogWidget(QWidget* parent = NULL);

        void printf(int blockType, const char* format, va_list args);
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