#pragma once

#include <vector>

#include <Nubuck\math\vector2.h>

#include <Nubuck\operators\operator_events.h>
#include <Nubuck\events\events.h>
#include <Nubuck\world\editmode.h>

class   QWidget;
struct  Nubuck;

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

    virtual void Invoke() { }

    virtual QWidget* GetWidget() { return _widget; }
};

class Operator : public EV::EventHandler<> {
private:
    OperatorPanel* _panel;
public:
    DECL_HANDLE_EVENTS(Operator);

    Operator() : _panel(NULL) { }

    virtual ~Operator() { }

    void SetPanel(OperatorPanel* panel); // called by operators manager only

    void SendToPanel(const EV::Event& event);

    virtual void Register(Invoker& invoker) = 0;
    virtual bool Invoke() = 0; // return false to decline invocation
    virtual void Finish() = 0;

    virtual void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) { }
    virtual void OnGeometrySelected() { }
    virtual void OnCameraChanged() { }
    virtual void OnEditModeChanged(const W::editMode_t::Enum mode) { }
	virtual bool OnMouse(const MouseEvent& mouseEvent) { return false; }
    virtual bool OnKey(const KeyEvent& keyEvent) { return false; }
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