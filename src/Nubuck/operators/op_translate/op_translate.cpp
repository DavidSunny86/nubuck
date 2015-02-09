#include <maxint.h>

#include <world\world.h>
#include "op_translate.h"

namespace OP {

void Translate::OpenEditor() {
    // first, close all editors
    _entityEditor.Close();
    _vertexEditor.Close();

    if(W::editMode_t::OBJECTS == _mode) {
        _entityEditor.Open();
    }
    else if(W::editMode_t::VERTICES == _mode) {
        NB::Mesh mesh = NB::FirstSelectedMesh();
        _vertexEditor.Open(mesh);
    } else {
        COM_assert(0 && "unkown editmode");
    }
}

Translate::Translate() : _mode(0) {
    _vertexEditor.SetModifyGlobalSelection(true);

    AddEventHandler(ev_usr_selectEntity, this, &Translate::Event_UsrSelectEntity);
    AddEventHandler(ev_usr_changeEditMode, this, &Translate::Event_UsrChangeEditMode);
	AddEventHandler(ev_w_selectionChanged, this, &Translate::Event_SelectionChanged);
}

void Translate::Register(Invoker& invoker) {
}

bool Translate::Invoke() {
    NB::SetOperatorName("Translate");

    _mode = W::world.GetEditMode().GetMode();
    OpenEditor();

    return true;
}

void Translate::OnGeometrySelected() {
    // well, that's the lazy solution.
    OpenEditor();
}

void Translate::Event_UsrSelectEntity(const EV::Usr_SelectEntity& event) {
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

void Translate::Event_UsrChangeEditMode(const EV::Arg<int>& event) {
    COM_assert(event.value != W::world.GetEditMode().GetMode());
    if(!event.IsFallthrough()) {
        W::world.GetEditMode().SetMode(W::editMode_t::Enum(event.value));
    }
    event.Accept();
}

void Translate::OnEditModeChanged(const W::editMode_t::Enum mode) {
    _mode = mode;
    OpenEditor();
}

void Translate::OnMouse(const EV::MouseEvent& event) {
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

void Translate::OnKey(const EV::KeyEvent& event) {
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