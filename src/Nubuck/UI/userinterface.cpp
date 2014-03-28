#include <nubuck_private.h>
#include <world\world_events.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\userinterface.h>

UserInterface g_ui;

void UserInterface::Event_EditModeChanged(const EV::Event& event) {
    const EV::Params_EditModeChanged& args = EV::def_EditModeChanged.GetArgs(event);
    UI::MainWindow* mainWindow = (UI::MainWindow*)nubuck.ui;
    mainWindow->ToolBar_UpdateEditMode(W::EditMode::Enum(args.editMode));
}

void UserInterface::Init() {
    AddEventHandler(EV::def_EditModeChanged, this, &UserInterface::Event_EditModeChanged);
}