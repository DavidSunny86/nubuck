#include <world\world.h>
#include "mainloop.h"

void MainLoop::Update() { W::world.Update(); }

MainLoop::MainLoop() { connect(&_timer, SIGNAL(timeout()), this, SLOT(Update())); }

void MainLoop::Enter() { _timer.start(); }