#include <QScrollArea>
#include <QDockWidget>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <events\event_defs.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\algorithmwidget\algorithmwidget.h>
#include <UI\logwidget\logwidget.h>
#include <UI\outliner\outliner.h>
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

        _renderConfig = new RenderConfig(this);
        _renderConfig->setFloating(true);
        _renderConfig->hide();
    }

} // namespace UI