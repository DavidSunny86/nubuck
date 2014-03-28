#pragma once

#include <QtGui\QMainwindow.h>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include <UI\renderconfig\renderconfig.h>
#include <world\world.h> // EditMode
#include "ui_mainwindow.h"

namespace UI {

	class MainWindow : public QMainWindow, public IMainWindow {
        Q_OBJECT
	private:
		Ui::MainWindow  _ui;
        RenderConfig*   _renderConfig;
        QDockWidget*    _outlinerDock;

        // toolbar
        QAction*    _editModeActs[W::editMode_t::NUM_MODES];
        void        ToolBar_Build();
    private slots:
        void        OnEditModeToObjects();
        void        OnEditModeToVertices();
    protected:
        void closeEvent(QCloseEvent*) override;
    public slots:
	    void OnRandomPoints(void);
        void OnShowRenderMetrics(void);
        void OnShowRenderConfig(void);
        void OnShowOutliner(void) { }
        void OnLoadOperatorPlugin();
    public:
		MainWindow(void);

        void ToolBar_UpdateEditMode(const W::editMode_t::Enum mode);

        QMenu* GetSceneMenu() override { return _ui.menuScene; }
        QMenu* GetObjectMenu() override { return _ui.menuObject; }
        QMenu* GetAlgorithmMenu() override { return _ui.menuAlgorithms; }
        void SetOperatorName(const char* name) override;
        void SetOperatorPanel(QWidget* widget) override;
	};

} // namespace UI