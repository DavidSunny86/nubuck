#pragma once

#include <QGroupBox>

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\editors\entity_editor.h>
#include <Nubuck\editors\vertex_editor.h>
#include <operators\operators.h>
#include <world\world_events.h>

// forward declarations
class NBW_SpinBox;

namespace OP {

class TransformPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private:
    QWidget*        _vectors;       // contains _grpPosition, _grpScale
    QGroupBox*      _grpPosition;
    QGroupBox*      _grpScale;
    NBW_SpinBox*    _sbPosition[3]; // xyz
    NBW_SpinBox* 	_sbScale[3];    // xyz

    void Event_ShowVectors(const EV::Arg<bool>& event);
    void Event_SetPosition(const EV::Arg<NB::Point3>& event);
private slots:
    void OnPositionChanged(leda::rational value);
public:
    TransformPanel();
};

class Transform : public Operator {
private:
    NB::EntityEditor _entityEditor;
    NB::VertexEditor _vertexEditor;

    int     _mode;
    bool    _isEditorOpen;

    void OpenEditor();

    void UpdatePanelVectorsVisibility();
    void UpdatePanelVectorsValues();

    void Event_UsrSelectEntity(const EV::Usr_SelectEntity& event);
    void Event_UsrChangeEditMode(const EV::Arg<int>& event);

    void Event_SelectionChanged(const EV::Event& event) {
        OnGeometrySelected();
	}

    void Event_SetPosition(const EV::Arg<NB::Point3>& event);
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
    void OnMeshChanged(const EV::Event& event) override;
};

} // namespace OP