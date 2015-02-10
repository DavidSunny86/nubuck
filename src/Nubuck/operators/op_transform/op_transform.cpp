#include <maxint.h>

#include <world\world.h>
#include "op_transform.h"

namespace OP {

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

Transform::Transform() : _mode(0), _isEditorOpen(false) {
    _vertexEditor.SetModifyGlobalSelection(true);

    AddEventHandler(ev_usr_selectEntity, this, &Transform::Event_UsrSelectEntity);
    AddEventHandler(ev_usr_changeEditMode, this, &Transform::Event_UsrChangeEditMode);
	AddEventHandler(ev_w_selectionChanged, this, &Transform::Event_SelectionChanged);
}

void Transform::Register(Invoker& invoker) {
}

bool Transform::Invoke() {
    NB::SetOperatorName("Transform");

    _mode = W::world.GetEditMode().GetMode();
    OpenEditor();

    return true;
}

void Transform::OnGeometrySelected() {
    // well, that's the lazy solution.
    OpenEditor();
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

} // namespace OP