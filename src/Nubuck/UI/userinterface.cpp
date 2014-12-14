#include <nubuck_private.h>
#include <world\world_events.h>
#include <UI\outliner\outliner.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\penoptions\pen_options.h>
#include <UI\userinterface.h>

UserInterface g_ui;

void UserInterface::Event_EditModeChanged(const EV::Arg<int>& event) {
    int editMode = event.value;
    _mainWindow->ToolBar_UpdateEditMode(W::editMode_t::Enum(editMode));
}

void UserInterface::OnQuit() {
    // freeing resources in dtor causes exceptions
    _outliner.Drop();
    _operatorPanel.Drop();
    _penOptions.Drop();
    _mainWindow.Drop();
}

void UserInterface::Init() {
    AddEventHandler(ev_w_editModeChanged, this, &UserInterface::Event_EditModeChanged);

    // order matters
    _outliner = GEN::MakePtr(new UI::Outliner());
    _operatorPanel = GEN::MakePtr(new UI::OperatorPanel());
    _penOptions = GEN::MakePtr(new UI::PenOptions());
    _mainWindow = GEN::MakePtr(new UI::MainWindow());

    _uiThreadID = SYS::Thread::CallerID();
}