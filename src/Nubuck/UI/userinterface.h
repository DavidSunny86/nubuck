#pragma once

#include <QObject>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\events\events.h>
#include <system\thread\thread.h>

// forward decls
namespace UI {

class Outliner;
class OperatorPanel;
class MainWindow;
class PenOptions;

} // namespace UI

class UserInterface : public QObject, public EV::EventHandler<> {
    Q_OBJECT
private:
    GEN::Pointer<UI::Outliner>      _outliner;
    GEN::Pointer<UI::OperatorPanel> _operatorPanel;
    GEN::Pointer<UI::PenOptions>    _penOptions;
    GEN::Pointer<UI::MainWindow>    _mainWindow;

    SYS::Thread::threadID_t _uiThreadID;

    void Event_EditModeChanged(const EV::Event& event);
public slots:
    void OnQuit();
public:
    UserInterface() : _uiThreadID(SYS::Thread::INVALID_THREAD_ID) { }

    SYS::Thread::threadID_t UI_ThreadID() const { return _uiThreadID; }

    void Init();

    const UI::MainWindow&       GetMainWindow() const { return *_mainWindow; }
    UI::MainWindow&             GetMainWindow() { return *_mainWindow; }

    const UI::OperatorPanel&    GetOperatorPanel() const { return *_operatorPanel; }
    UI::OperatorPanel&          GetOperatorPanel() { return *_operatorPanel; }

    const UI::Outliner&         GetOutliner() const { return *_outliner; }
    UI::Outliner&               GetOutliner() { return *_outliner; }

    const UI::PenOptions&       GetPenOptions() const { return *_penOptions; }
    UI::PenOptions&             GetPenOptions() { return *_penOptions; }

    DECL_HANDLE_EVENTS(UserInterface);
};

extern UserInterface g_ui;