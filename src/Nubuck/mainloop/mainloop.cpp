#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\renderview\renderview.h>
#include <UI\logwidget\logwidget.h>
#include <operators\operators.h>
#include <world\world.h>
#include "mainloop.h"

void MainLoop::Update() { 
	OP::g_operators.FrameUpdate();
    g_ui.GetOutliner().HandleEvents();
    g_ui.HandleEvents();
	W::world.Update(); 

    UI::RenderView* renderView = g_ui.GetMainWindow().GetRenderView();
    renderView->Use();
    renderView->Render();

    UI::LogWidget::Instance()->Flush();
}

MainLoop::MainLoop() { connect(&_timer, SIGNAL(timeout()), this, SLOT(Update())); }

void MainLoop::Enter() { _timer.start(); }