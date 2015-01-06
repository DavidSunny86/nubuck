#include <Nubuck\vertex_editor.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>
#include "ent_geometry.h"

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

static M::Vector3 CenterOfVertexSelection(const W::ENT_Geometry& geom) {
    const std::vector<leda::node> verts = geom.GetVertexSelection();
    COM_assert(!verts.empty());
    M::Vector3 pos = M::Vector3::Zero;
    const M::Matrix4 objToWorld = geom.GetObjectToWorldMatrix();
    for(unsigned i = 0; i < verts.size(); ++i) {
        pos += M::Transform(objToWorld, ToVector(geom.GetRatPolyMesh().position_of(verts[i])));
    }
    float d = 1.0f / verts.size();
    return d * pos;
}

namespace NB {

bool VertexEditor::DoPicking(const OP::MouseEvent& event, W::ENT_Geometry& geom) {
    if(OP::MouseEvent::MOUSE_DOWN == event.type && OP::MouseEvent::BUTTON_RIGHT == event.button) {
        M::Ray ray = W::world.PickingRay(event.coords);
        std::vector<W::ENT_Geometry::VertexHit> hits;
        if(geom.TraceVertices(ray, 0.2f, hits)) {
            // find nearest hit
            unsigned nidx = 0;
            for(unsigned i = 1; i < hits.size(); ++i) {
                if(hits[nidx].dist > hits[i].dist)
                    nidx = i;
            }

            Nubuck::SelectMode selectMode = Nubuck::SELECT_MODE_ADD;
            if(0 == (OP::MouseEvent::MODIFIER_SHIFT & event.mods)) {
                W::world.SelectVertex_New(&geom, hits[nidx].vert);
            } else {
                W::world.SelectVertex_Add(&geom, hits[nidx].vert);
            }

            return true;
        }
    }

    return false;
}

void VertexEditor::OnBeginDragging(W::ENT_Geometry& geom) {
    const leda::nb::RatPolyMesh& mesh = geom.GetRatPolyMesh();

    const std::vector<leda::node> verts = geom.GetVertexSelection();

    // save old positions of selected vertices
    _oldVertPosF.init(mesh);
    _oldVertPosR.init(mesh);
    for(unsigned i = 0; i < verts.size(); ++i) {
        const leda::node v = verts[i];
        _oldVertPosR[v] = mesh.position_of(v);
        _oldVertPosF[v] = ToVector(_oldVertPosR[v]);
    }

    // compute center
    _center = M::Vector3::Zero;
    for(unsigned i = 0; i < verts.size(); ++i)
        _center += _oldVertPosF[verts[i]];
    _center /= verts.size();
}

void VertexEditor::OnDragging(const Nubuck::transform_gizmo_mouse_info& info, W::ENT_Geometry& geom) {
    leda::nb::RatPolyMesh& mesh = geom.GetRatPolyMesh();

    const std::vector<leda::node> verts = geom.GetVertexSelection();
    COM_assert(!verts.empty());

    for(unsigned i = 0; i < verts.size(); ++i) {
        const leda::node v = verts[i];
        M::Vector3 pos = _oldVertPosF[v];
        pos.vec[info.axis] = _oldVertPosF[v].vec[info.axis] + info.value;
        mesh.set_position(v, ToRatPoint(pos));
    }
}

VertexEditor::VertexEditor() : _axis(nb::AF_XYZ), _gizmo(NULL) {
    _gizmo = W::world.CreateTransformGizmo();
    _gizmo->HideCursor();
}

void VertexEditor::SetAxisFlags(int axisFlags) {
    _axis = axisFlags;
    _gizmo->SetAxis(_axis);
}

bool VertexEditor::HandleMouseEvent(const OP::MouseEvent& event, W::ENT_Geometry* geom) {
    COM_assert(geom);

    bool retval = false;

    Nubuck::transform_gizmo_mouse_info mouseInfo;
    if(_gizmo->HandleMouseEvent(event, mouseInfo)) {
        if(Nubuck::transform_gizmo_action::BEGIN_DRAGGING == mouseInfo.action) {
            OnBeginDragging(*geom);
        }
        if(Nubuck::transform_gizmo_action::DRAGGING == mouseInfo.action) {
            OnDragging(mouseInfo, *geom);
        }

        retval = true;
    } else {
        retval = DoPicking(event, *geom);
    }

    // _gizmo->SetAxis(_axis);
    if(geom->GetVertexSelection().empty()) {
        _gizmo->HideCursor();
    } else {
        _gizmo->ShowCursor();
        _gizmo->SetCursorPosition(CenterOfVertexSelection(*geom));
    }

    return retval;
}

} // namespace NB