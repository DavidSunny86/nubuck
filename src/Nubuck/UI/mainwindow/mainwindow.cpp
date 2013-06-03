#include <QScrollArea>
#include <QDockWidget>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <world\entities\ent_node\ent_node.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\algorithmwidget\algorithmwidget.h>
#include <UI\logwidget\logwidget.h>
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

    void MainWindow::OnRandomPoints(void) {
        RandomPoints randomPoints;
        if(QDialog::Accepted == randomPoints.exec()) {
			W::Event event;
			event.id = W::EVENT_APOCALYPSE;
			event.sem = NULL;
			W::world.Send(event);

            ALG::Driver& algDrv = ALG::Driver::Instance();
            algDrv.Init(randomPoints.AsGraph());
        }
    }

    void MainWindow::OnShowRenderMetrics(void) {
        RenderMetrics::Instance()->show();
    }

    MainWindow::MainWindow(void) {
        _ui.setupUi(this);

        RenderView* renderView = new RenderView();
        statusBar()->addWidget(renderView->FpsLabel());
        setCentralWidget(renderView);

        AlgorithmWidget* algorithmWidget = new AlgorithmWidget();
        addDockWidget(Qt::RightDockWidgetArea, ScrollableDockWidget("Algorithm", algorithmWidget));

        UI::LogWidget* logWidget = UI::LogWidget::Instance();
        addDockWidget(Qt::RightDockWidgetArea, ScrollableDockWidget("LogWidget", logWidget));
    }

} // namespace UI