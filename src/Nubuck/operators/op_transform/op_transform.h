#pragma once

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\editors\entity_editor.h>
#include <Nubuck\editors\vertex_editor.h>
#include <operators\operators.h>
#include <world\world_events.h>

namespace OP {

class TransformPanel : public OperatorPanel {
public:
    TransformPanel() { }
};

class Transform : public Operator {
private:
    NB::EntityEditor _entityEditor;
    NB::VertexEditor _vertexEditor;

    int _mode;

    void OpenEditor();

    void Event_UsrSelectEntity(const EV::Usr_SelectEntity& event);
    void Event_UsrChangeEditMode(const EV::Arg<int>& event);

    void Event_SelectionChanged(const EV::Event& event) {
        OnGeometrySelected();
	}
public:
    Transform();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }

    void OnGeometrySelected() override;
    void OnEditModeChanged(const W::editMode_t::Enum mode) override;
    void OnMouse(const EV::MouseEvent& event) override;
    void OnKey(const EV::KeyEvent& event) override;
};

} // namespace OP