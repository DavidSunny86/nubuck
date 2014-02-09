#include <QFileDialog>
#include <QScrollArea>
#include <QDockWidget>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <events\event_defs.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\algorithmwidget\algorithmwidget.h>
#include <UI\logwidget\logwidget.h>
#include <UI\outliner\outliner.h>
#include <UI\operatorpanel\operatorpanel.h>
#include "mainwindow.h"

namespace {

    QDockWidget* ScrollableDockWidget(const QString& title, QWidget* widget) {
        QScrollArea* scrollArea = new QScrollArea();
        QDockWidget* dockWidget = new QDockWidget(title);

        scrollArea->setWidget(widget);
        scrollArea->setWidgetResizable(true);
        dockWidget->setWidget(scrollArea);

        return dockWidget;
    }

} // unnamed namespace

namespace UI {

    void MainWindow::closeEvent(QCloseEvent*) {
        qApp->exit();
    }

    void MainWindow::OnRandomPoints(void) {
        RandomPoints randomPoints;
        if(QDialog::Accepted == randomPoints.exec()) {
            W::world.Send(EV::def_Apocalypse.Create(EV::Params_Apocalypse()));

            ALG::gs_algorithm.Init(randomPoints.AsGraph());
        }
    }

    void MainWindow::OnShowRenderMetrics(void) {
        RenderMetrics::Instance()->show();
    }

    void MainWindow::OnShowRenderConfig(void) {
        _renderConfig->show();
    }

	void MainWindow::OnLoadOperatorPlugin() {
        QString filename = QFileDialog::getOpenFileName(
            this,
            "Choose an operator plugin file",
			QDir::currentPath(),
			"Operator Plugins (*.dll)");
		if(!filename.isNull()) {
			HMODULE lib = LoadLibraryA(filename.toAscii());
            if(!lib) {
				printf("ERROR: unable to load '%s'\n", filename.toAscii());
			}
			typedef OP::Operator* (*createOperator_t)();
            createOperator_t func = (createOperator_t)GetProcAddress(lib, "CreateOperator");
            if(!func) printf("ERROR - unable to load createoperator() function\n");
			else {
				OP::Operator* op = func();
				OP::g_operators.Register(op, lib);
			}
		}
	}

    MainWindow::MainWindow(void) {
        _ui.setupUi(this);

        RenderView* renderView = new RenderView();
        statusBar()->addWidget(renderView->FpsLabel());
        setCentralWidget(renderView);

        AlgorithmWidget* algorithmWidget = new AlgorithmWidget();
        QDockWidget* algorithmDock = ScrollableDockWidget("Algorithm", algorithmWidget);
        addDockWidget(Qt::RightDockWidgetArea, algorithmDock);

        _outlinerDock = ScrollableDockWidget("Outliner", Outliner::Instance());
        addDockWidget(Qt::RightDockWidgetArea, _outlinerDock);

        tabifyDockWidget(_outlinerDock, algorithmDock);

        UI::LogWidget* logWidget = UI::LogWidget::Instance();
        addDockWidget(Qt::RightDockWidgetArea, ScrollableDockWidget("LogWidget", logWidget));

        addDockWidget(Qt::LeftDockWidgetArea, OperatorPanel::Instance());

        _renderConfig = new RenderConfig(this);
        _renderConfig->setFloating(true);
        _renderConfig->hide();
    }

    void MainWindow::SetOperatorName(const char* name) {
        OperatorPanel::Instance()->setWindowTitle(QString("Operator ") + name);
    }

    void MainWindow::SetOperatorPanel(QWidget* panel) {
        OperatorPanel::Instance()->setWidget(panel);
    }

} // namespace UI