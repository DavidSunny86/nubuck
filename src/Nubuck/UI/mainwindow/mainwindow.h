#pragma once

#include <QtGui\QMainwindow.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include "ui_mainwindow.h"

namespace UI {

	class MainWindow : public QMainWindow {
        Q_OBJECT
	private:
		Ui::MainWindow _ui;
    protected:
        void closeEvent(QCloseEvent*) override;
    public slots:
	    void OnRandomPoints(void);
        void OnShowRenderMetrics(void);
        void OnShowFaceConfig(void);
        void OnShowNodeConfig(void);
    public:
		MainWindow(void);
	};

} // namespace UI