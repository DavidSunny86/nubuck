#include <nubuck_private.h>
#include <world\world_events.h>
#include <UI\outliner\outliner.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\userinterface.h>

UserInterface g_ui;

void UserInterface::Event_EditModeChanged(const EV::Event& event) {
    const EV::Params_EditModeChanged& args = EV::def_EditModeChanged.GetArgs(event);
    _mainWindow->ToolBar_UpdateEditMode(W::editMode_t::Enum(args.editMode));
}

void UserInterface::OnQuit() {
    // freeing resources in dtor causes exceptions
    _outliner.Drop();
    _operatorPanel.Drop();
    _mainWindow.Drop();
}

void UserInterface::Init() {
    AddEventHandler(EV::def_EditModeChanged, this, &UserInterface::Event_EditModeChanged);

    // order matters
    _outliner = GEN::MakePtr(new UI::Outliner());
    _operatorPanel = GEN::MakePtr(new UI::OperatorPanel());
    _mainWindow = GEN::MakePtr(new UI::MainWindow());
}