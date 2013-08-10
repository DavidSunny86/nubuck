#include <QScrollArea>
#include <QDockWidget>

#include <nubuck_private.h>
#include <algdriver\algdriver.h>
#include <UI\renderview\renderview.h>
#include <UI\rendermetrics\rendermetrics.h>
#include <UI\randompoints\randompoints.h>
#include <UI\algorithmwidget\algorithmwidget.h>
#include <UI\logwidget\logwidget.h>
#include <UI\faceconfig\faceconfig.h>
#include <UI\nodeconfig\nodeconfig.h>
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
            ALG::Driver& algDrv = ALG::Driver::Instance();
            algDrv.Init(randomPoints.AsGraph());
        }
    }

    void MainWindow::OnShowRenderMetrics(void) {
        RenderMetrics::Instance()->show();
    }

    void MainWindow::OnShowFaceConfig(void) { FaceConfig::Show(); }
    void MainWindow::OnShowNodeConfig(void) { NodeConfig::Show(); }

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