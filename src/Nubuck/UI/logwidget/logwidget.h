#pragma once

#include <QWidget>
#include <Nubuck\nubuck.h>
#include "ui_logwidget.h"

namespace UI {

    class LogWidget : public QWidget, public ILog {
        Q_OBJECT
    private:
        Ui::LogWidget   _ui;
        bool            _enabled;

        LogWidget(QWidget* parent = NULL);
    private slots:
        void OnSetEnabled(bool enable);
    public:
        static LogWidget* Instance(void);

        bool IsEnabled(void) const;

        void Enable(void);
        void Disable(void);

        void printf(const char* format, ...) override;
    };

} // namespace UI

#include "logwidget_inl.h"