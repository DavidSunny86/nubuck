#include <UI\outliner\outliner.h>
#include <world\world.h>
#include "mainloop.h"

void MainLoop::Update() { 
	W::world.Update(); 
	UI::Outliner::Instance()->HandleEvents();
}

MainLoop::MainLoop() { connect(&_timer, SIGNAL(timeout()), this, SLOT(Update())); }

void MainLoop::Enter() { _timer.start(); }