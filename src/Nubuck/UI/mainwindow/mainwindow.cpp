#include <QToolBar>
#include <QToolButton>
#include <QFileDialog>
#include <QScrollArea>
#include <QDockWidget>
#include <QColorDialog>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <world\world_events.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <UI\block_signals.h>
#include <UI\console\console.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\logwidget\logwidget.h>
#include <UI\outliner\outliner.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <UI\penoptions\pen_options.h>
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

        for(unsigned i = 0; i < NUM_MENUS; ++i) toolBar->addAction(_opMenus[i]->action);

        addToolBar(Qt::TopToolBarArea, toolBar);

        ToolBar_UpdateEditMode(W::editMode_t::DEFAULT);
    }

    void MainWindow::ToolBar_UpdateEditMode(const W::editMode_t::Enum mode) {
        for(int i = 0; i < W::editMode_t::NUM_MODES; ++i) {
            bool isEnabled = mode == W::editMode_t::Enum(i);
            BlockSignals blockSigs(_editModeActs[i]);
            _editModeActs[i]->setChecked(isEnabled);
        }

        for(unsigned i = 0; i < NUM_MENUS; ++i) {
            _opMenus[i]->action->setVisible(_opMenus[i]->mode == mode);
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

    void MainWindow::OnToggleRenderViewControls() {
        ToggleRenderViewControls();
    }

    void MainWindow::OnShowConsole() {
        _console->show();
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

    void MainWindow::OnShowOperator() {
        _operatorDock->show();
    }

    void MainWindow::OnShowOutliner() {
        _outlinerDock->show();
    }

    void MainWindow::OnShowLog() {
        _logDock->show();
    }

    void MainWindow::OnShowPenOptions() {
        _penOptionsDock->show();
    }

    MainWindow::OperatorMenu::OperatorMenu(QWidget* parent, const W::editMode_t::Enum mode, const QString& name) {
        this->mode = mode;
        menu = new QMenu(name);
        button = new QToolButton();
        button->setObjectName("menuButton");
        button->setText(name);
        button->setMenu(menu);
        button->setPopupMode(QToolButton::InstantPopup);
        action = new QWidgetAction(parent);
        action->setDefaultWidget(button);
    }

    void MainWindow::OperatorMenus_Build() {
        _opMenus[MENU_SCENE]        = GEN::MakePtr(new OperatorMenu(this, W::editMode_t::OBJECTS,    "Scene"));
        _opMenus[MENU_OBJECT]       = GEN::MakePtr(new OperatorMenu(this, W::editMode_t::OBJECTS,    "Object"));
        _opMenus[MENU_ALGORITHMS]   = GEN::MakePtr(new OperatorMenu(this, W::editMode_t::OBJECTS,    "Algorithms"));
        _opMenus[MENU_VERTEX]       = GEN::MakePtr(new OperatorMenu(this, W::editMode_t::VERTICES,   "Vertex"));
    }

    MainWindow::MainWindow(void) {
        _ui.setupUi(this);

        _console = new Console();

        OperatorMenus_Build();
        ToolBar_Build();

        _renderView = new RenderView();
        statusBar()->addWidget(_renderView->FpsLabel());
        setCentralWidget(_renderView);

        _outlinerDock = ScrollableDockWidget("Outliner", &g_ui.GetOutliner());
        addDockWidget(Qt::RightDockWidgetArea, _outlinerDock);

        UI::LogWidget* logWidget = UI::LogWidget::Instance();
        _logDock = ScrollableDockWidget("LogWidget", logWidget);
        addDockWidget(Qt::RightDockWidgetArea, _logDock);

        _operatorDock = ScrollableDockWidget("Operator", &g_ui.GetOperatorPanel());
        addDockWidget(Qt::LeftDockWidgetArea, _operatorDock);

        _penOptionsDock = ScrollableDockWidget("Pen Options", &g_ui.GetPenOptions());
        _penOptionsDock->setParent(this);
        _penOptionsDock->setFloating(true);
        _penOptionsDock->hide();

        _renderConfig = new RenderConfig(_renderView, this);
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