#include <Nubuck\editors\vertex_editor.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>
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
====================
UpdateGizmo

    places gizmo at center of vertex selection in world space, and
    hides it, if selection is empty
====================
*/
void VertexEditor::UpdateGizmo() {
    COM_assert(_subject);
    COM_assert(_selection.get_owner() == &_subject->GetRatPolyMesh());

    const leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    M::Vector3 accu = M::Vector3::Zero;
    unsigned numSelected = 0;
    leda::node v;
    forall_nodes(v, mesh) {
        if(_selection[v]) {
            accu += ToVector(mesh.position_of(v));
            numSelected++;
        }
    }

    float d = 1.0f / numSelected;
    const M::Vector3 center_ws = M::Transform(_subject->GetObjectToWorldMatrix(), d * accu);
    SetGizmoPosition(center_ws);    
    SetGizmoVisibility(0 < numSelected);
}

bool VertexEditor::DoPicking(const EV::MouseEvent& event) {
    COM_assert(_subject);

    if(EV::MouseEvent::MOUSE_DOWN == event.type && EV::MouseEvent::BUTTON_RIGHT == event.button) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        std::vector<W::ENT_Geometry::VertexHit> hits;
        if(_subject->TraceVertices(ray, 0.2f, hits)) {
            // find nearest hit
            unsigned nidx = 0;
            for(unsigned i = 1; i < hits.size(); ++i) {
                if(hits[nidx].dist > hits[i].dist)
                    nidx = i;
            }

            leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

            if(0 == (EV::MouseEvent::MODIFIER_SHIFT & event.mods)) {
                _selection.init(mesh, false);
            }
            _selection[hits[nidx].vert] = true;

            W::SetColorsFromVertexSelection(mesh, _selection, _col_unselected, _col_selected);
            UpdateGizmo();

            return true;
        }
    }

    return false;
}

void VertexEditor::OnBeginDragging() {
    COM_assert(_subject);

    const leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    // save old vertex positions
    _oldVertPosF.init(mesh);
    _oldVertPosR.init(mesh);

    leda::node v;
    forall_nodes(v, mesh) {
        _oldVertPosR[v] = mesh.position_of(v);
        _oldVertPosF[v] = ToVector(_oldVertPosR[v]);
    }
}

void VertexEditor::OnDragging() {
    leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

    int dragAxis = GetDragAxis();
    float translation = GetTranslation();

    leda::node v;
    forall_nodes(v, mesh) {
        if(_selection[v]) {
            M::Vector3 pos = _oldVertPosF[v];
            pos.vec[dragAxis] = _oldVertPosF[v].vec[dragAxis] + translation;
            mesh.set_position(v, ToRatPoint(pos));
        }
    }

    UpdateGizmo();
}

bool VertexEditor::OnMouseEvent(const EV::MouseEvent& event) {
    return DoPicking(event);
}

VertexEditor::VertexEditor()
    : _col_unselected(R::Color::Black)
    , _col_selected(R::Color::Yellow)
    , _subject(NULL)
{ }

void VertexEditor::Open(W::ENT_Geometry* geom) {
    COM_assert(geom);

    _subject = geom;

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    _selection.init(mesh, false);
    _oldColors.init(mesh);
    _oldVertPosF.init(mesh);
    _oldVertPosR.init(mesh);

    leda::node v;
    forall_nodes(v, mesh) {
        _oldColors[v] = mesh.color_of(v);
        // vertex positions assigned later
    }

    W::SetColorsFromVertexSelection(mesh, _selection, _col_unselected, _col_selected);
    UpdateGizmo();
}

void VertexEditor::Close() {
}

} // namespace NB