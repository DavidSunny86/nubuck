#pragma once

#include <QObject>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\events\events.h>

// forward decls
namespace UI {

class Outliner;
class OperatorPanel;
class MainWindow;

} // namespace UI

class UserInterface : public QObject, public EV::EventHandler<> {
    Q_OBJECT
private:
    GEN::Pointer<UI::Outliner>      _outliner;
    GEN::Pointer<UI::OperatorPanel> _operatorPanel;
    GEN::Pointer<UI::MainWindow>    _mainWindow;

    void Event_EditModeChanged(const EV::Event& event);
public slots:
    void OnQuit();
public:
    void Init();

    const UI::MainWindow&       GetMainWindow() const { return *_mainWindow; }
    UI::MainWindow&             GetMainWindow() { return *_mainWindow; }

    const UI::OperatorPanel&    GetOperatorPanel() const { return *_operatorPanel; }
    UI::OperatorPanel&          GetOperatorPanel() { return *_operatorPanel; }

    const UI::Outliner&         GetOutliner() const { return *_outliner; }
    UI::Outliner&               GetOutliner() { return *_outliner; }

    DECL_HANDLE_EVENTS(UserInterface);
};

extern UserInterface g_ui;