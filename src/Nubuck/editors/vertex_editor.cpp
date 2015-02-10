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
==================================================
TranslateImpl implementation {
==================================================
*/

VertexEditor::TranslateImpl::TranslateImpl(VertexEditor* vertexEditor) : vertexEditor(vertexEditor) { }

void VertexEditor::TranslateImpl::OnDragging() {
    leda::nb::RatPolyMesh& mesh = vertexEditor->GetSubject()->GetRatPolyMesh();

    int dragAxis = vertexEditor->GetDragAxis();
    float translation = vertexEditor->GetTranslation();

    leda::node v;
    forall_nodes(v, mesh) {
        if(vertexEditor->IsSelected(v)) {
            M::Vector3 pos = vertexEditor->GetOldVertexPosition(v);
            pos.vec[dragAxis] = vertexEditor->GetOldVertexPosition(v).vec[dragAxis] + translation;
            mesh.set_position(v, ToRatPoint(pos));
        }
    }

    vertexEditor->UpdateGizmo();
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

VertexEditor::ScaleImpl::ScaleImpl(VertexEditor* vertexEditor) : vertexEditor(vertexEditor) { }

void VertexEditor::ScaleImpl::OnDragging() {
    W::ENT_Geometry* geom = vertexEditor->GetSubject();
    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    int dragAxis = vertexEditor->GetDragAxis();
    float scale = vertexEditor->GetScale();

    const M::Vector3& center = vertexEditor->GetCenterOfSelection();

    leda::node v;
    forall_nodes(v, mesh) {
        if(vertexEditor->IsSelected(v)) {
            M::Vector3 pos = vertexEditor->GetOldVertexPosition(v) - center;
            pos.vec[dragAxis] *= scale;
            pos += center;
            mesh.set_position(v, ToRatPoint(pos));
        }
    }

    vertexEditor->UpdateGizmo();
}

/*
==================================================
} ScaleImpl implementation
==================================================
*/

void VertexEditor::ClearSelection() {
    _selection.init(_subject->GetRatPolyMesh(), false);
    _numSelected = 0;

    if(_modifyGlobalSelection) {
        _subject->ClearVertexSelection();
    }
}

void VertexEditor::Select(leda::node v) {
    if(!_selection[v]) {
        _selection[v] = true;
        _numSelected++;

        if(_modifyGlobalSelection) {
            _subject->Select(v);
        }
    }
}

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

bool VertexEditor::DoPicking(const EV::MouseEvent& event, bool simulate) {
    COM_assert(_subject);

    if(EV::MouseEvent::MOUSE_DOWN == event.type && EV::MouseEvent::BUTTON_RIGHT == event.button) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        std::vector<W::ENT_Geometry::VertexHit> hits;
        if(_subject->TraceVertices(ray, 0.2f, hits)) {
            if(!simulate) {
                // find nearest hit
                unsigned nidx = 0;
                for(unsigned i = 1; i < hits.size(); ++i) {
                    if(hits[nidx].dist > hits[i].dist)
                        nidx = i;
                }

                leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();

                if(0 == (EV::MouseEvent::MODIFIER_SHIFT & event.mods)) {
                    ClearSelection();
                }
                Select(hits[nidx].vert);

                W::SetColorsFromVertexSelection(mesh, _selection, _col_unselected, _col_selected);
                UpdateGizmo();
            } // if(!simulate)

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

    _center = M::Vector3::Zero;
    leda::node v;
    forall_nodes(v, mesh) {
        _oldVertPosR[v] = mesh.position_of(v);
        _oldVertPosF[v] = ToVector(_oldVertPosR[v]);

        if(_selection[v]) _center += _oldVertPosF[v];
    }
    
    _center /= _numSelected;
}

void VertexEditor::OnDragging() {
    _curImpl->OnDragging();
}

bool VertexEditor::OnMouseEvent(const EV::MouseEvent& event, bool simulate) {
    return DoPicking(event, simulate);
}

bool VertexEditor::OnKeyEvent(const EV::KeyEvent& event, bool simulate) {
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

    if('A' == event.keyCode) {
        if(!simulate && !event.autoRepeat) {
            COM_assert(_subject);
            leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();
            if(_numSelected < mesh.number_of_nodes()) {
                _selection.init(mesh, true);
                _numSelected = mesh.number_of_nodes();

                leda::node v;
                forall_nodes(v, mesh) {
                    _subject->Select(v);
                }
            } else ClearSelection();

            W::SetColorsFromVertexSelection(mesh, _selection, _col_unselected, _col_selected);
            UpdateGizmo();
        }
        return true;
    }

    return false;
}

VertexEditor::VertexEditor()
    : _curImpl(NULL)
    , _allowedModeFlags(TMF_ALL)
    , _mode(0)
    , _modifyGlobalSelection(false)
    , _numSelected(0)
    , _col_unselected(R::Color::Black)
    , _col_selected(R::Color::Yellow)
    , _subject(NULL)
{
    _impl[0] = GEN::MakePtr(new TranslateImpl(this));
    _impl[1] = GEN::MakePtr(new ScaleImpl(this));
    _curImpl = _impl[0].Raw();
}

void VertexEditor::SetAllowedModeFlags(int flags) {
    _allowedModeFlags = flags;
}

void VertexEditor::SetMode(int mode) {
    if(!(_allowedModeFlags & 1 << mode)) {
        COM_printf("WARNING - VertexEditor::SetMode(): required mode not allowed\n");
        return;
    }
    _mode = mode;
    SetGizmoTransformMode(NB::TransformGizmoMode(_mode));
    _curImpl = _impl[_mode].Raw();
}

void VertexEditor::SetModifyGlobalSelection(bool modify) {
    _modifyGlobalSelection = modify;
}

void VertexEditor::Open(W::ENT_Geometry* geom) {
    COM_assert(geom);

    _subject = geom;

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    _selection.init(mesh, false);
    _numSelected = 0;

    if(_modifyGlobalSelection) {
        std::vector<leda::node> vertSel = _subject->GetVertexSelection();
        ClearSelection(); // note that vertex selection of geom is cleared at this point, too
        for(unsigned i = 0; i < vertSel.size(); ++i) {
            Select(vertSel[i]);
        }
    }

    _oldVertColors.init(mesh);
    _oldEdgeColors.init(mesh);
    _oldVertPosF.init(mesh);
    _oldVertPosR.init(mesh);

    leda::node v;
    forall_nodes(v, mesh) {
        _oldVertColors[v] = mesh.color_of(v);
        // vertex positions assigned later
    }

    leda::edge e;
    forall_edges(e, mesh) {
        _oldEdgeColors[e] = mesh.color_of(e);
    }

    W::SetColorsFromVertexSelection(mesh, _selection, _col_unselected, _col_selected);
    UpdateGizmo();
}

void VertexEditor::Close() {
    SetGizmoVisibility(false);

    leda::nb::RatPolyMesh& mesh = _subject->GetRatPolyMesh();
    if(_oldVertColors.get_owner() == &mesh) { // HACK: have colors been initialized?
        COM_assert(_oldEdgeColors.get_owner() == &mesh);

        leda::node v;
        forall_nodes(v, mesh) {
            mesh.set_color(v, _oldVertColors[v]);
        }

        leda::edge e;
        forall_edges(e, mesh) {
            mesh.set_color(e, _oldEdgeColors[e]);
        }
    }
}

} // namespace NB