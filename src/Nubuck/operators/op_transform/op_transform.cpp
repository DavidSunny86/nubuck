#include <maxint.h>

#include <Nubuck\UI\nbw_spinbox.h>
#include <Nubuck\math_conv.h>
#include <UI\block_signals.h>
#include <world\world.h>
#include "op_transform.h"

EV::ConcreteEventDef<EV::Arg<bool> >        ev_showVectors;
EV::ConcreteEventDef<EV::Arg<NB::Point3> >  ev_setPosition;

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
}

/*
==================================================
    Transform Implementation
==================================================
*/

void Transform::OpenEditor() {
    // first, close all editors
    _entityEditor.Close();
    _vertexEditor.Close();
    _isEditorOpen = false;

    if(W::editMode_t::OBJECTS == _mode) {
        _entityEditor.Open();
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
    if(W::editMode_t::OBJECTS == _mode) {
        if(NB::EntityEditor::Mode_Translate == _entityEditor.GetMode()) {
            M::Vector3 center = _entityEditor.GlobalCenterOfSelection();
            event.value = ToRatPoint(center);
            SendToPanel(ev_setPosition.Tag(event));
        } else if(NB::EntityEditor::Mode_Scale == _entityEditor.GetMode()) {
        }
    }
}

Transform::Transform() : _mode(0), _isEditorOpen(false) {
    _entityEditor.SetModifyGlobalSelection(true);
    _vertexEditor.SetModifyGlobalSelection(true);

    AddEventHandler(ev_usr_selectEntity, this, &Transform::Event_UsrSelectEntity);
    AddEventHandler(ev_usr_changeEditMode, this, &Transform::Event_UsrChangeEditMode);
	AddEventHandler(ev_w_selectionChanged, this, &Transform::Event_SelectionChanged);
    AddEventHandler(ev_setPosition, this, &Transform::Event_SetPosition);
}

void Transform::Register(Invoker& invoker) {
}

bool Transform::Invoke() {
    NB::SetOperatorName("Transform");

    _mode = W::world.GetEditMode().GetMode();
    OpenEditor();
    UpdatePanelVectorsVisibility();

    return true;
}

void Transform::OnGeometrySelected() {
    if(W::editMode_t::OBJECTS == _mode) {
        _entityEditor.CopyGlobalSelection();
        UpdatePanelVectorsVisibility();
    }
}

void Transform::Event_UsrSelectEntity(const EV::Usr_SelectEntity& event) {
    COM_assert(event.entity);
    if(!event.IsFallthrough()) {
        if(event.shiftModifier) {
            W::world.Select_Add(event.entity);
        } else {
            W::world.Select_New(event.entity);
        }
    }
    event.Accept();
}

void Transform::Event_UsrChangeEditMode(const EV::Arg<int>& event) {
    COM_assert(event.value != W::world.GetEditMode().GetMode());
    if(!event.IsFallthrough()) {
        W::world.GetEditMode().SetMode(W::editMode_t::Enum(event.value));
    }
    event.Accept();
}

void Transform::Event_SetPosition(const EV::Arg<NB::Point3>& event) {
    if(W::editMode_t::OBJECTS == _mode) {
        // move all selected entities relative to global center of selection
        M::Vector3 oldCenter = _entityEditor.GlobalCenterOfSelection();
        M::Vector3 center = ToVector(event.value);
        M::Vector3 translation = center - oldCenter;

        W::Entity* ent = _entityEditor.FirstSelectedEntity();
        while(ent) {
            ent->SetPosition(ent->GetPosition() + translation);
            ent = _entityEditor.NextSelectedEntity(ent);
        }

        _entityEditor.UpdateBoundingBoxes();
    }

    event.Accept();
}

void Transform::OnEditModeChanged(const W::editMode_t::Enum mode) {
    _mode = mode;
    OpenEditor();
}

void Transform::OnMouse(const EV::MouseEvent& event) {
    if(!_isEditorOpen) return;

    bool accept = false;
    if(W::editMode_t::OBJECTS == _mode) {
        if(event.fallthrough) {
            accept = _entityEditor.SimulateMouseEvent(event);
        } else {
            accept = _entityEditor.HandleMouseEvent(event); 
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
        _entityEditor.CopyGlobalSelection(); // takes care of deletion
        _entityEditor.UpdateBoundingBoxes();
    } else {
        // HACK, should listen to selectionChanged event instead.
        COM_assert(W::editMode_t::VERTICES == _mode);
        _vertexEditor.UpdateGizmo();
    }
}

} // namespace OP