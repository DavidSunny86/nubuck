#pragma once

#include <string>
#include <vector>

#include <Nubuck\nubuck.h>
#include <Nubuck\math\vector2.h>

#include <Nubuck\events\events.h>
#include <Nubuck\world\editmode.h>

class   QWidget;
struct  Nubuck;

namespace EV {

struct MouseEvent;
struct KeyEvent;

} // namespace EV

namespace R {

struct MeshJob;

} // namespace R

namespace OP {

class Invoker;

class NUBUCK_API OperatorPanel : public EV::EventHandler<> {
private:
    QWidget* _widget;
public:
    DECL_HANDLE_EVENTS(OperatorPanel);

    OperatorPanel();

    virtual ~OperatorPanel() { }

    void SetLayout(NB::BoxLayout layout);

    virtual void Invoke() { }

    virtual QWidget* GetWidget() { return _widget; }
};

class NUBUCK_API Operator : public EV::EventHandler<> {
private:
    OperatorPanel* _panel;

    // TODO must be consistent with buffer size of SetOperatorEvent
    enum { ARGS_BUFFER_SIZE = 128 };
    char _args[128];
public:
    DECL_HANDLE_EVENTS(Operator);

    Operator();

    virtual ~Operator() { }

    // called by operators manager only
    void SetPanel(OperatorPanel* panel);
    void SetArgumentData(const char* args);

    void SendToPanel(const EV::Event& event);

    const char* GetArgumentData() const;
    int         GetArgumentDataSize() const;

    virtual std::string PreferredShortcut() const;

    virtual void Register(Invoker& invoker) = 0;
    virtual bool Invoke() = 0; // return false to decline invocation
    virtual void Finish() = 0;
    virtual bool IsDone() const { return false; }

    virtual void OnMouse(const EV::MouseEvent& mouseEvent) { }
    virtual void OnKey(const EV::KeyEvent& keyEvent) { }
    virtual void OnMeshChanged(const EV::Event& event) { }
    virtual void OnRequestFinish(const EV::Event& event);
};

NUBUCK_API void SendToOperator(const EV::Event& event);

NUBUCK_API void WaitForAnimations();

inline void Operator::SetPanel(OperatorPanel* panel) {
    _panel = panel;
}

inline void Operator::SendToPanel(const EV::Event& event) {
    assert(_panel);
    _panel->Send(event);
}

} // namespace OP