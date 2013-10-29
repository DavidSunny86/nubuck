#pragma once

#include <QtGui\QMainwindow.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include <UI\renderconfig\renderconfig.h>
#include "ui_mainwindow.h"

namespace UI {

	class MainWindow : public QMainWindow {
        Q_OBJECT
	private:
		Ui::MainWindow  _ui;
        RenderConfig*   _renderConfig;
        QDockWidget*    _outlinerDock;
    protected:
        void closeEvent(QCloseEvent*) override;
    public slots:
	    void OnRandomPoints(void);
        void OnShowRenderMetrics(void);
        void OnShowRenderConfig(void);
        void OnShowOutliner(void) { }
    public:
		MainWindow(void);
	};

} // namespace UI