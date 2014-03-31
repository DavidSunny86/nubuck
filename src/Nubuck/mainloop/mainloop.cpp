#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\renderview\renderview.h>
#include <operators\operators.h>
#include <world\world.h>
#include "mainloop.h"

void MainLoop::Update() { 
	OP::g_operators.FrameUpdate();
    g_ui.GetOutliner().HandleEvents();
    g_ui.HandleEvents();
	W::world.Update(); 

    g_ui.GetMainWindow().GetRenderView()->Render();
}

MainLoop::MainLoop() { connect(&_timer, SIGNAL(timeout()), this, SLOT(Update())); }

void MainLoop::Enter() { _timer.start(); }