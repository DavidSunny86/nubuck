#pragma once

#include <QTimer>
#include <QWidget>
#include "ui_rendermetrics.h"

namespace UI {

    class RenderMetrics : public QWidget {
        Q_OBJECT
    private:
        Ui::RenderMetrics _ui;
        QTimer _timer;

        RenderMetrics(QWidget* parent = NULL);
    private slots:
        void Update(void);
    public:
        static RenderMetrics* Instance(void);
    };

} // namespace UI