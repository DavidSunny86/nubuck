#pragma once

#include <QWidget>
#include "ui_algorithm.h"

namespace UI {

    class AlgorithmWidget : public QWidget {
        Q_OBJECT
    private:
        Ui::AlgorithmWidget _ui;
    private slots:
        void OnStep(void);
        void OnNext(void);
        void OnRun(void);
        void OnReset(void);
    public:
        AlgorithmWidget(QWidget* parent = NULL);
    };

} // namespace UI