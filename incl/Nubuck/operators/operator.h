#pragma once

#include <vector>
#include <QWidget>

#include <Nubuck\math\vector2.h>

#include <Nubuck\operators\mouse_event.h>
#include <Nubuck\events\events.h>
#include <Nubuck\world\editmode.h>

struct Nubuck;

namespace R {

struct MeshJob;

} // namespace R

namespace OP {

class Invoker;

class OperatorPanel : public QWidget, public EV::EventHandler<> {
public:
    OperatorPanel(QWidget* parent = 0) : QWidget(parent) { }
    virtual ~OperatorPanel() { }

    virtual void Invoke() { }
};

class Operator : public EV::EventHandler<> {
public:
    DECL_HANDLE_EVENTS(Operator);

    virtual ~Operator() { }

    virtual void Register(const Nubuck& nb, Invoker& invoker) = 0;
    virtual void Invoke() = 0;
    virtual void Finish() = 0;

    virtual void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) { }
    virtual void OnGeometrySelected() { }
    virtual void OnCameraChanged() { }
    virtual void OnEditModeChanged(const W::editMode_t::Enum mode) { }
	virtual bool OnMouse(const MouseEvent& mouseEvent) { return false; }
};

NUBUCK_API void SendToOperator(const EV::Event& event);

} // namespace OP