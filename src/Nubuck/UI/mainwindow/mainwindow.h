#pragma once

#include <QtGui\QMainwindow.h>
#include <QWidgetAction>

#include <Nubuck\nubuck.h>
#include <common\types.h>
#include <UI\renderconfig\renderconfig.h>
#include <world\world.h> // EditMode
#include "ui_mainwindow.h"

namespace UI {

    class Console;
    class RenderView;

	class MainWindow : public QMainWindow, public IMainWindow {
        Q_OBJECT
	private:
		Ui::MainWindow  _ui;
        Console*        _console;
        RenderView*     _renderView;
        RenderConfig*   _renderConfig;
        QDockWidget*    _outlinerDock;

        // operator menus
        struct OperatorMenu {
            W::editMode_t::Enum mode;
            QMenu*              menu;
            QToolButton*    	button;
            QWidgetAction*  	action;

            OperatorMenu(QWidget* parent, const W::editMode_t::Enum mode, const QString& name);
        };
        enum { MENU_SCENE = 0, MENU_OBJECT, MENU_ALGORITHMS, MENU_VERTEX, NUM_MENUS };
        GEN::Pointer<OperatorMenu> _opMenus[NUM_MENUS];
        void OperatorMenus_Build();


        // toolbar
        QAction*    _editModeActs[W::editMode_t::NUM_MODES];
        void        ToolBar_Build();
    private slots:
        void        OnEditModeToObjects();
        void        OnEditModeToVertices();
    protected:
        void closeEvent(QCloseEvent*) override;
    public slots:
        void OnToggleRenderViewControls();
        void OnShowConsole();
	    void OnRandomPoints(void);
        void OnShowRenderMetrics(void);
        void OnShowRenderConfig(void);
        void OnShowOutliner(void) { }
        void OnLoadOperatorPlugin();
    public:
		MainWindow(void);

        void ToolBar_UpdateEditMode(const W::editMode_t::Enum mode);

        RenderView* GetRenderView() { return _renderView; }

        QMenu* GetSceneMenu() override      { return _opMenus[MENU_SCENE]->menu; }
        QMenu* GetObjectMenu() override     { return _opMenus[MENU_OBJECT]->menu; }
        QMenu* GetAlgorithmMenu() override  { return _opMenus[MENU_ALGORITHMS]->menu; }
        QMenu* GetVertexMenu() override     { return _opMenus[MENU_VERTEX]->menu; }

        void SetOperatorName(const char* name) override;
        void SetOperatorPanel(QWidget* widget) override;
	};

} // namespace UI