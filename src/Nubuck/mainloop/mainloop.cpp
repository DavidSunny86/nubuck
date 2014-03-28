#include <UI\outliner\outliner.h>
#include <UI\userinterface.h>
#include <operators\operators.h>
#include <world\world.h>
#include "mainloop.h"

void MainLoop::Update() { 
	W::world.Update(); 
	OP::g_operators.FrameUpdate();
	UI::Outliner::Instance()->HandleEvents();
    g_ui.HandleEvents();
}

MainLoop::MainLoop() { connect(&_timer, SIGNAL(timeout()), this, SLOT(Update())); }

void MainLoop::Enter() { _timer.start(); }