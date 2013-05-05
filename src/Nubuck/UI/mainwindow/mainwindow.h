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
    public slots:
	    void OnRandomPoints(void);
    public:
		MainWindow(void);
	};

} // namespace UI