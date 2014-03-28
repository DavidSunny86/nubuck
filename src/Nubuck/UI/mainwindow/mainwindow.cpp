#include <QToolBar>
#include <QFileDialog>
#include <QScrollArea>
#include <QDockWidget>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <world\world_events.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <UI\block_signals.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\logwidget\logwidget.h>
#include <UI\outliner\outliner.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <UI\userinterface.h>
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

    void MainWindow::ToolBar_Build() {
        struct {
            const char* icon;
            const char* text;
            const char* recv;
        } descs[] = {
            { ":/ui/Images/faces.png",      "Edit Objects",  SLOT(OnEditModeToObjects()) },
            { ":/ui/Images/vertices.png",   "Edit Vertices", SLOT(OnEditModeToVertices()) }
        };

        QToolBar* toolBar = new QToolBar("toolbar");
        toolBar->setIconSize(QSize(15, 15));
        toolBar->addWidget(new QLabel("edit mode:"));
        for(int i = 0; i < W::editMode_t::NUM_MODES; ++i) {
            _editModeActs[i] = toolBar->addAction(QIcon(descs[i].icon), descs[i].text, this, descs[i].recv);
            _editModeActs[i]->setCheckable(true);
        }
        toolBar->addSeparator();
        addToolBar(toolBar);

        ToolBar_UpdateEditMode(W::editMode_t::DEFAULT);
    }

    void MainWindow::ToolBar_UpdateEditMode(const W::editMode_t::Enum mode) {
        for(int i = 0; i < W::editMode_t::NUM_MODES; ++i) {
            bool isEnabled = mode == W::editMode_t::Enum(i);
            BlockSignals blockSigs(_editModeActs[i]);
            _editModeActs[i]->setChecked(isEnabled);
        }
    }

    void MainWindow::OnEditModeToObjects() { 
        W::world.GetEditMode().SetMode(W::editMode_t::OBJECTS);
    }

    void MainWindow::OnEditModeToVertices() { 
        W::world.GetEditMode().SetMode(W::editMode_t::VERTICES);
    }

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
            typedef OP::OperatorPanel* (*createPanel_t)();
            createOperator_t opFunc = (createOperator_t)GetProcAddress(lib, "CreateOperator");
            createPanel_t panelFunc = (createPanel_t)GetProcAddress(lib, "CreateOperatorPanel");
            if(!opFunc || !panelFunc) printf("ERROR - unable to load createoperator() function\n");
			else {
				OP::Operator* op = opFunc();
                OP::OperatorPanel* panel = panelFunc();
				OP::g_operators.Register(panel, op, lib);
			}
		}
	}

    MainWindow::MainWindow(void) {
        _ui.setupUi(this);

        ToolBar_Build();

        RenderView* renderView = new RenderView();
        statusBar()->addWidget(renderView->FpsLabel());
        setCentralWidget(renderView);

        _outlinerDock = ScrollableDockWidget("Outliner", &g_ui.GetOutliner());
        addDockWidget(Qt::RightDockWidgetArea, _outlinerDock);

        UI::LogWidget* logWidget = UI::LogWidget::Instance();
        addDockWidget(Qt::RightDockWidgetArea, ScrollableDockWidget("LogWidget", logWidget));

        addDockWidget(Qt::LeftDockWidgetArea, &g_ui.GetOperatorPanel());

        _renderConfig = new RenderConfig(this);
        _renderConfig->setFloating(true);
        _renderConfig->hide();
    }

    void MainWindow::SetOperatorName(const char* name) {
        g_ui.GetOperatorPanel().SetOperatorName(name);
    }

    void MainWindow::SetOperatorPanel(QWidget* panel) {
        g_ui.GetOperatorPanel().setWidget(panel);
    }

} // namespace UI