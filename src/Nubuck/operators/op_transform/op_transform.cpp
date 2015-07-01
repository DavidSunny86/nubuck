#include <maxint.h>

#include <Nubuck\UI\nbw_spinbox.h>
#include <Nubuck\math_conv.h>
#include <UI\block_signals.h>
#include <world\world.h>
#include "op_transform.h"

EV::ConcreteEventDef<EV::Arg<bool> >        ev_showVectors;
EV::ConcreteEventDef<EV::Arg<NB::Point3> >  ev_setPosition;
EV::ConcreteEventDef<EV::Arg<NB::Point3> >  ev_setScale;

namespace OP {

/*
==================================================
    TransformPanel Implementation
==================================================
*/

void TransformPanel::Event_ShowVectors(const EV::Arg<bool>& event) {
    _vectors->setVisible(event.value);
    event.Accept();
}

void TransformPanel::Event_SetPosition(const EV::Arg<NB::Point3>& event) {
    {
        UI::BlockSignals blockSignals(
            _sbPosition[0],
            _sbPosition[1],
            _sbPosition[2]);
        _sbPosition[0]->setValue(event.value.xcoord());
        _sbPosition[1]->setValue(event.value.ycoord());
        _sbPosition[2]->setValue(event.value.zcoord());
    }
    event.Accept();
}

void TransformPanel::Event_SetScale(const EV::Arg<NB::Point3>& event) {
    {
        UI::BlockSignals blockSignals(
            _sbScale[0],
            _sbScale[1],
            _sbScale[2]);
        _sbScale[0]->setValue(event.value.xcoord());
        _sbScale[1]->setValue(event.value.ycoord());
        _sbScale[2]->setValue(event.value.zcoord());
    }
    event.Accept();
}

void TransformPanel::OnPositionChanged(leda::rational) {
    NB::Point3 position(
        _sbPosition[0]->value(),
        _sbPosition[1]->value(),
        _sbPosition[2]->value());
    SendToOperator(ev_setPosition.Tag(position));
}

TransformPanel::TransformPanel() {

    // create gui

    QVBoxLayout* vbox = NULL;

    _grpPosition = new QGroupBox("position");
    _grpPosition->setObjectName("vectorGroup");

    vbox = new QVBoxLayout;
    for(int i = 0; i < 3; ++i) {
        _sbPosition[i] = new NBW_SpinBox;
        _sbPosition[i]->setText(QString("%1: ").arg(static_cast<char>('x' + i)));
        // TODO: actually, it's not good that these values are bounded
        _sbPosition[i]->setMinimum(-100);
        _sbPosition[i]->setMaximum( 100);
        _sbPosition[i]->setSingleStep(leda::rational(1, 10));
        connect(_sbPosition[i], SIGNAL(SigValueChanged(leda::rational)), this, SLOT(OnPositionChanged(leda::rational)));
        vbox->addWidget(_sbPosition[i]);
    }
    _grpPosition->setLayout(vbox);

    _grpScale = new QGroupBox("scale");
    _grpScale->setObjectName("vectorGroup");

    vbox = new QVBoxLayout;
    for(int i = 0; i < 3; ++i) {
        _sbScale[i] = new NBW_SpinBox;
        _sbScale[i]->setText(QString("%1: ").arg(static_cast<char>('x' + i)));
        // TODO: actually, it's not good that these values are bounded
        _sbScale[i]->setMinimum(-100);
        _sbScale[i]->setMaximum( 100);
        _sbScale[i]->setSingleStep(leda::rational(1, 10));
        vbox->addWidget(_sbScale[i]);
    }
    _grpScale->setLayout(vbox);

    vbox = new QVBoxLayout;
    vbox->addWidget(_grpPosition);
    vbox->addWidget(_grpScale);
    vbox->addStretch();

    _vectors = new QWidget;
    _vectors->setLayout(vbox);
    _vectors->hide();

    vbox = new QVBoxLayout;
    vbox->addWidget(_vectors);

    SetLayout(vbox);

    // set event handlers

    AddEventHandler(ev_showVectors, this, &TransformPanel::Event_ShowVectors);
    AddEventHandler(ev_setPosition, this, &TransformPanel::Event_SetPosition);
    AddEventHandler(ev_setScale, this, &TransformPanel::Event_SetScale);
}

void TransformPanel::Invoke() {
    {
        UI::BlockSignals blockSignals(_sbPosition[0], _sbPosition[1], _sbPosition[2]);
        _sbPosition[0]->setValue(0);
        _sbPosition[1]->setValue(0);
        _sbPosition[2]->setValue(0);
    }
    {
        UI::BlockSignals blockSignals(_sbScale[0], _sbScale[1], _sbScale[2]);
        _sbScale[0]->setValue(1);
        _sbScale[1]->setValue(1);
        _sbScale[2]->setValue(1);
    }
}

/*
==================================================
    Transform Implementation
==================================================
*/

void Transform::SetEditMode(int mode) {
    if(mode != _mode) {
        _mode = mode;
        OpenEditor();
        UpdatePanelVectorsVisibility();
        UpdatePanelVectorsValues();
    }
}

void Transform::OpenEditor() {
    // first, close all editors
    _entityEditor.Close();
    _vertexEditor.Close();
    _isEditorOpen = false;

    if(W::editMode_t::OBJECTS == _mode) {
        _entityEditor.Open();
        _entityEditor.CopyGlobalSelection();
        _isEditorOpen = true;
    }
    else if(W::editMode_t::VERTICES == _mode) {
        NB::Mesh mesh = NB::FirstSelectedMesh();
        if(mesh) {
            _vertexEditor.Open(mesh);
            _isEditorOpen = true;
        }
    } else {
        COM_assert(0 && "unkown editmode");
    }
}

void Transform::CopyEditorSelection() {
    // update global selection
    int lastAction = _entityEditor.GetAction();
    if( NB::EntityEditor::Action_SelectEntity == lastAction ||
        NB::EntityEditor::Action_SelectEntity_Add == lastAction)
    {
        std::vector<W::Entity*> selection;
        W::Entity* ent = _entityEditor.FirstSelectedEntity();
        while(ent) {
            selection.push_back(ent);
            ent = _entityEditor.NextSelectedEntity(ent);
        }
        if(selection.empty()) {
            W::world.ClearSelection();
        } else {
            W::world.Select_InArray(&selection[0], selection.size());
        }
    }
}

// hides the position-scale vector groups of the panel if the current
// selection is empty, and shows them otherwise
void Transform::UpdatePanelVectorsVisibility() {
    bool showVectors =
        (W::editMode_t::OBJECTS == _mode && _entityEditor.FirstSelectedEntity()) ||
        (W::editMode_t::VERTICES == _mode && _vertexEditor.FirstSelectedVertex());
    SendToPanel(ev_showVectors.Tag(showVectors));
}

void Transform::UpdatePanelVectorsValues() {
    EV::Arg<NB::Point3> event;
    if(W::editMode_t::OBJECTS == _mode && _entityEditor.FirstSelectedEntity()) {
        if(NB::EntityEditor::Mode_Translate == _entityEditor.GetMode()) {
            event.value = ToRatPoint(_entityEditor.GetTranslationVector());
            SendToPanel(ev_setPosition.Tag(event));
        } else if(NB::EntityEditor::Mode_Scale == _entityEditor.GetMode()) {
            M::Vector3 scalingVector = _entityEditor.GetScalingVector();
            event.value = ToRatPoint(scalingVector);
            SendToPanel(ev_setScale.Tag(event));
        }
    }
}

Transform::Transform() : _mode(0), _isEditorOpen(false) {
    _vertexEditor.SetModifyGlobalSelection(true);

    AddEventHandler(ev_usr_selectEntity, this, &Transform::Event_UsrSelectEntity);
    AddEventHandler(ev_usr_changeEditMode, this, &Transform::Event_UsrChangeEditMode);
	AddEventHandler(ev_w_selectionChanged, this, &Transform::Event_SelectionChanged);
    AddEventHandler(ev_w_editModeChanged, this, &Transform::Event_EditModeChanged);
    AddEventHandler(ev_setPosition, this, &Transform::Event_SetPosition);
}

void Transform::Register(Invoker& invoker) {
}

bool Transform::Invoke() {
    NB::SetOperatorName("Transform");

    _mode = W::world.GetEditMode().GetMode();
    OpenEditor();
    UpdatePanelVectorsVisibility();
    UpdatePanelVectorsValues();

    return true;
}

void Transform::Event_SelectionChanged(const EV::Event& event) {
    /*
    If this is not a fallthrough event, then we set the selection
    ourselves and we do not need to update the editor selection
    */
    if(event.IsFallthrough()) {
        if(W::editMode_t::OBJECTS == _mode) {
            _entityEditor.CopyGlobalSelection();
            UpdatePanelVectorsVisibility();
            UpdatePanelVectorsValues();
        }
    }
}

void Transform::Event_EditModeChanged(const EV::Arg<int>& event) {
    if(event.IsFallthrough()) {
        SetEditMode(event.value);
    }
    event.Accept();
}

void Transform::Event_UsrSelectEntity(const EV::Usr_SelectEntity& event) {
    COM_assert(event.entity);
    if(!event.IsFallthrough()) {
        SetEditMode(0); // TODO: magic number
        if(event.shiftModifier) {
            _entityEditor.SelectEntity_Add(event.entity);
        } else {
            _entityEditor.SelectEntity_New(event.entity);
        }
        CopyEditorSelection();
    }
    event.Accept();
}

void Transform::Event_UsrChangeEditMode(const EV::Arg<int>& event) {
    if(!event.IsFallthrough()) {
        SetEditMode(event.value);
        W::world.GetEditMode().SetMode(W::editMode_t::Enum(event.value));
    }
    event.Accept();
}

void Transform::Event_SetPosition(const EV::Arg<NB::Point3>& event) {
    if(W::editMode_t::OBJECTS == _mode) {
        _entityEditor.SetTranslationVector(ToVector(event.value));
    }

    event.Accept();
}

void Transform::OnMouse(const EV::MouseEvent& event) {
    if(!_isEditorOpen) return;

    bool accept = false;
    if(W::editMode_t::OBJECTS == _mode) {
        if(event.fallthrough) {
            accept = _entityEditor.SimulateMouseEvent(event);
        } else {
            accept = _entityEditor.HandleMouseEvent(event);
            if(accept) {
                CopyEditorSelection();
                UpdatePanelVectorsValues();
            }
        }
    } else if(W::editMode_t::VERTICES == _mode) {
        if(event.fallthrough) {
            accept = _vertexEditor.SimulateMouseEvent(event);
        } else {
            accept = _vertexEditor.HandleMouseEvent(event);
        }
    } else {
        COM_assert(0 && "unknown editmode");
    }
    if(accept) event.Accept();
}

void Transform::OnKey(const EV::KeyEvent& event) {
    if(!_isEditorOpen) return;

    bool accept = false;
    if(W::editMode_t::OBJECTS == _mode) {
        if(event.fallthrough) {
            accept = _entityEditor.SimulateKeyEvent(event);
        } else {
            accept = _entityEditor.HandleKeyEvent(event);
            if(accept) {
                CopyEditorSelection();
                UpdatePanelVectorsValues();
            }
        }
    } else if(W::editMode_t::VERTICES == _mode) {
        if(event.fallthrough) {
            accept = _vertexEditor.SimulateKeyEvent(event);
        } else {
            accept = _vertexEditor.HandleKeyEvent(event);
        }
    } else {
        COM_assert(0 && "unknown editmode");
    }
    if(accept) event.Accept();

}

void Transform::OnMeshChanged(const EV::Event& event) {
    if(W::editMode_t::OBJECTS == _mode) {
        // takes care of deletions
        bool updateSelection = false;
        W::Entity* ent = _entityEditor.FirstSelectedEntity();
        while(ent) {
            if(ent->IsDead()) updateSelection = true;
            ent = _entityEditor.NextSelectedEntity(ent);
        }
        if(updateSelection) {
            _entityEditor.ClearSelection();
            _entityEditor.CopyGlobalSelection();
        }

        _entityEditor.UpdateBoundingBoxes();
    } else {
        // HACK, should listen to selectionChanged event instead.
        COM_assert(W::editMode_t::VERTICES == _mode);
        _vertexEditor.UpdateGizmo();
    }
}

} // namespace OP