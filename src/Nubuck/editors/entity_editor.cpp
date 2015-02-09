#include <Nubuck\editors\entity_editor.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>

static M::Vector3 ToVector(const leda::d3_rat_point& p) { // TODO: duplicate of ent_geometry.cpp:ToVector
    const leda::d3_point fp = p.to_float();
    return M::Vector3(fp.xcoord(), fp.ycoord(), fp.zcoord());
}

static leda::d3_rat_point ToRatPoint(const M::Vector3& v) {
    const leda::rational x = v.x;
    const leda::rational y = v.y;
    const leda::rational z = v.z;
    return leda::d3_rat_point(x, y, z);
}

namespace NB {

/*
==================================================
TranslateImpl implementation {
==================================================
*/

EntityEditor::TranslateImpl::TranslateImpl(EntityEditor* entityEditor) : entityEditor(entityEditor) { }

void EntityEditor::TranslateImpl::OnBeginDragging() {
    oldEntPos.clear();
    W::Entity* ent = W::world.FirstSelectedEntity();
    while(ent) {
        oldEntPos.push_back(ent->GetPosition());
        ent = W::world.NextSelectedEntity(ent);
    }
}

void EntityEditor::TranslateImpl::OnDragging() {
    int dragAxis = entityEditor->GetDragAxis();
    float translation = entityEditor->GetTranslation();

    W::Entity* ent = W::world.FirstSelectedEntity();
    unsigned i = 0;
    while(ent) {
        M::Vector3 pos = oldEntPos[i];
        pos.vec[dragAxis] = oldEntPos[i].vec[dragAxis] + translation;
        ent->SetPosition(pos);
        ent = W::world.NextSelectedEntity(ent);
        i++;
    }

    entityEditor->UpdateGizmo();
}

/*
==================================================
} TranslateImpl implementation
==================================================
*/

/*
==================================================
ScaleImpl implementation {
==================================================
*/

EntityEditor::ScaleImpl::ScaleImpl(EntityEditor* entityEditor) : entityEditor(entityEditor) { }

void EntityEditor::ScaleImpl::OnBeginDragging() {
    W::ENT_Geometry* geom = NB::FirstSelectedMesh(); // naming confusion, oh boy
    if(geom) {
        const leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        oldVertPosF.init(mesh);

        leda::node v;
        forall_nodes(v, mesh) {
            oldVertPosF[v] = ToVector(mesh.position_of(v));
        }
    }
}

void EntityEditor::ScaleImpl::OnDragging() {
    int dragAxis = entityEditor->GetDragAxis();
    float scale = entityEditor->GetScale();

    // TODO: this is broken, does not work for multiple selected geometry objects
    W::ENT_Geometry* geom = NB::FirstSelectedMesh();
    if(geom) {
        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        leda::node v;
        forall_nodes(v, mesh) {
            M::Vector3 pos = oldVertPosF[v];
            pos.vec[dragAxis] *= scale;
            mesh.set_position(v, ToRatPoint(pos));
        }
    }
}

/*
==================================================
} ScaleImpl implementation
==================================================
*/

/*
====================
UpdateGizmo

    places gizmo at center of vertex selection in world space, and
    hides it, if selection is empty
====================
*/
void EntityEditor::UpdateGizmo() {
    SetGizmoPosition(W::world.GlobalCenterOfSelection());
    SetGizmoVisibility(W::world.FirstSelectedEntity());
}

bool EntityEditor::DoPicking(const EV::MouseEvent& event, bool simulate) {
    if(EV::MouseEvent::MOUSE_DOWN == event.type && EV::MouseEvent::BUTTON_RIGHT == event.button) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        W::Entity* ent = NULL;
        if(W::world.TraceEntity(ray, &ent)) {
            if(!simulate) {
                if(EV::MouseEvent::MODIFIER_SHIFT & event.mods) W::world.Select_Add(ent);
                else W::world.Select_New(ent);
                UpdateGizmo();
            }
            return true;
        }
    }
    return false;
}

void EntityEditor::OnBeginDragging() {
    _curImpl->OnBeginDragging();
}

void EntityEditor::OnDragging() {
    _curImpl->OnDragging();
}

bool EntityEditor::OnMouseEvent(const EV::MouseEvent& event, bool simulate) {
    return DoPicking(event, simulate);
}

bool EntityEditor::OnKeyEvent(const EV::KeyEvent& event, bool simulate) {
    // scancodes for number row of generic usb keyboard
    static const unsigned numrow[3] = { 11, 2, 3 };

    if(numrow[1] == event.nativeScanCode) {
        if(!simulate) SetMode(0);
        return true;
    }
    if(numrow[2] == event.nativeScanCode) {
        if(!simulate) SetMode(1);
        return true;
    }

    return false;
}

EntityEditor::EntityEditor() : _curImpl(NULL), _allowedModeFlags(TMF_ALL), _mode(0) {
    _impl[0] = GEN::MakePtr(new TranslateImpl(this));
    _impl[1] = GEN::MakePtr(new ScaleImpl(this));
    _curImpl = _impl[0].Raw();
}

void EntityEditor::SetAllowedModeFlags(int flags) {
    _allowedModeFlags = flags;
}

void EntityEditor::SetMode(int mode) {
    if(!(_allowedModeFlags & 1 << mode)) {
        COM_printf("WARNING - EntityEditor::SetMode(): required mode not allowed.\n");
        return;
    }

    _mode = mode;
    SetGizmoTransformMode(NB::TransformGizmoMode(_mode));
    _curImpl = _impl[_mode].Raw();
}

void EntityEditor::Open() {
    UpdateGizmo();
}

void EntityEditor::Close() {
    SetGizmoVisibility(false);
}

} // namespace NB