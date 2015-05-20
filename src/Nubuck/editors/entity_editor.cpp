#include <Nubuck\math_conv.h>
#include <Nubuck\editors\entity_editor.h>
#include <Nubuck\face_vertex_mesh.h>
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
    W::Entity* ent = entityEditor->FirstSelectedEntity();
    while(ent) {
        oldEntPos.push_back(ent->GetPosition());
        ent = entityEditor->NextSelectedEntity(ent);
    }
}

void EntityEditor::TranslateImpl::OnDragging() {
    int dragAxis = entityEditor->GetDragAxis();
    float translation = entityEditor->GetTranslation();

    W::Entity* ent = entityEditor->FirstSelectedEntity();
    unsigned i = 0;
    while(ent) {
        M::Vector3 pos = oldEntPos[i];
        pos.vec[dragAxis] = oldEntPos[i].vec[dragAxis] + translation;
        ent->SetPosition(pos);

        // move bbox
        EntityData& data = entityEditor->_entData[ent->GetID()];
        COM_assert(data.bbox.geom);
        data.bbox.geom->SetPosition(pos);

        ent = entityEditor->NextSelectedEntity(ent);
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

static void MakeCube(leda::nb::RatPolyMesh& polyMesh) {
	leda::list<leda::d3_rat_point> positions;
	positions.push_back(leda::d3_rat_point(-1, -1, -1));
	positions.push_back(leda::d3_rat_point(-1, -1,  1));
	positions.push_back(leda::d3_rat_point(-1,  1, -1));
	positions.push_back(leda::d3_rat_point(-1,  1,  1));
	positions.push_back(leda::d3_rat_point( 1, -1, -1));
	positions.push_back(leda::d3_rat_point( 1, -1,  1));
	positions.push_back(leda::d3_rat_point( 1,  1, -1));
	positions.push_back(leda::d3_rat_point( 1,  1,  1));

    leda::list<unsigned> indices;
    add_quad(indices, 0, 1, 3, 2);
    add_quad(indices, 1, 5, 7, 3);
    add_quad(indices, 5, 4, 6, 7);
    add_quad(indices, 0, 2, 6, 4);
    add_quad(indices, 0, 4, 5, 1);
    add_quad(indices, 3, 7, 6, 2);

    make_from_indices(positions, indices, polyMesh);
}

// create bounding box topology, does not initialize vertex positions
void EntityEditor::BBox::Init() {
    geom = W::world.CreateGeometry();

    geom->SetName("bbox()");
    geom->SetRenderMode(NB::RM_EDGES);
    geom->SetShadingMode(NB::SM_LINES);
    geom->SetSolid(false);
    geom->HideOutline();

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    // TODO: simplify this
    MakeCube(mesh);
    int vidx = 0;
    leda::node v;
    forall_nodes(v, mesh) {
        verts[vidx++] = v;
    }

    leda::edge e;
    forall_edges(e, mesh) {
        mesh.set_color(e, R::Color::White);
    }
}

void EntityEditor::BBox::Update(W::Entity* ent) {
    const M::Box& mbox = ent->GetBoundingBox();

    const M::Vector3 boundary = 0.2f * M::Vector3(1.0f, 1.0f, 1.0f);

    const M::Vector3 min = mbox.min - boundary;
    const M::Vector3 max = mbox.max + boundary;

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
    mesh.set_position(verts[0], ToRatPoint(M::Vector3(min.x, min.y, min.z)));
    mesh.set_position(verts[1], ToRatPoint(M::Vector3(min.x, min.y, max.z)));
    mesh.set_position(verts[2], ToRatPoint(M::Vector3(min.x, max.y, min.z)));
    mesh.set_position(verts[3], ToRatPoint(M::Vector3(min.x, max.y, max.z)));
    mesh.set_position(verts[4], ToRatPoint(M::Vector3(max.x, min.y, min.z)));
    mesh.set_position(verts[5], ToRatPoint(M::Vector3(max.x, min.y, max.z)));
    mesh.set_position(verts[6], ToRatPoint(M::Vector3(max.x, max.y, min.z)));
    mesh.set_position(verts[7], ToRatPoint(M::Vector3(max.x, max.y, max.z)));

    geom->SetPosition(ent->GetPosition());

    std::stringstream name;
    name << "bbox(" << ent->GetID() << ")";
    geom->SetName(name.str());
}

void EntityEditor::BBox::Destroy() {
    if(geom) {
        geom->Destroy();
        geom = NULL;
    }
    for(int i = 0; i < 8; ++i) {
        verts[i] = NULL;
    }
}

bool EntityEditor::IsSelected(const W::Entity* ent) const {
    return _entData.size() > ent->GetID() && _entData[ent->GetID()].isSelected;
}

void EntityEditor::ClearSelection() {
    for(unsigned i = 0; i < _entData.size(); ++i) {
        _entData[i].bbox.Destroy();
    }
    _entData.clear();
    _selected = NULL;

    if(_modifyGlobalSelection) W::world.ClearSelection();
}

void EntityEditor::SelectEntity_Add(W::Entity* ent) {
    /* DEBUG */ std::cout << "EntityEditor::SelectEntity_Add(W::Entity* ent)" << std::endl;
    if(!IsSelected(ent)) {
        _entData.resize(M::Max(ent->GetID() + 1, _entData.size()));

        EntityData& data = _entData[ent->GetID()];

        data.isSelected = true;
        data.bbox.Init();
        data.bbox.Update(ent);

        // append ent to selection list
        W::Entity** tail = &_selected;
        while(*tail) {
            COM_assert(IsSelected(*tail));
            tail = &_entData[(*tail)->GetID()].nextSelected;
        }
        *tail = ent;

        if(_modifyGlobalSelection) W::world.Select_Add(ent);
    }
}

/*
====================
UpdateGizmo

    places gizmo at center of vertex selection in world space, and
    hides it, if selection is empty
====================
*/
void EntityEditor::UpdateGizmo() {
    SetGizmoPosition(GlobalCenterOfSelection());
    SetGizmoVisibility(NULL != FirstSelectedEntity());
}

bool EntityEditor::DoPicking(const EV::MouseEvent& event, bool simulate) {
    if(EV::MouseEvent::MOUSE_DOWN == event.type && EV::MouseEvent::BUTTON_RIGHT == event.button) {
        M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));
        W::Entity* ent = NULL;
        if(W::world.TraceEntity(ray, &ent)) {
            if(!simulate) {
                if(!(EV::MouseEvent::MODIFIER_SHIFT & event.mods)) ClearSelection();
                SelectEntity_Add(ent);
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
    
    if('A' == event.keyCode) {
        if(!simulate && !event.autoRepeat) {
            W::Entity* ent = NULL;

            bool isSelectionComplete = true;
            ent = W::world.FirstEntity();
            while(ent && isSelectionComplete) {
                isSelectionComplete &= !ent->IsSolid() || IsSelected(ent);
                ent = W::world.NextEntity(ent);
            }

            if(isSelectionComplete) ClearSelection();
            else {
                ent = W::world.FirstEntity();
                while(ent) {
                    if(ent->IsSolid()) SelectEntity_Add(ent);
                    ent = W::world.NextEntity(ent);
                }
            }

            UpdateGizmo();
        }
        return true;
    }

    return false;
}

EntityEditor::EntityEditor()
    : _curImpl(NULL)
    , _selected(NULL)
    , _allowedModeFlags(TMF_ALL)
    , _mode(0)
    , _modifyGlobalSelection(false)
{
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

void EntityEditor::SetModifyGlobalSelection(bool modify) {
    if(_modifyGlobalSelection = modify) {
        W::world.ClearSelection();
        W::Entity* ent = FirstSelectedEntity();
        while(ent) {
            W::world.Select_Add(ent);
            ent = NextSelectedEntity(ent);
        }
    }
}

void EntityEditor::Open() {
    UpdateGizmo();
}

void EntityEditor::Close() {
    SetGizmoVisibility(false);
}

// update
void EntityEditor::UpdateBoundingBoxes() {
    W::Entity* it = FirstSelectedEntity();
    while(it) {
        if(W::EntityType::ENT_GEOMETRY == it->GetType()) {
            W::ENT_Geometry* geom = static_cast<W::ENT_Geometry*>(it);
            if(geom->IsDirty()) {
                geom->CacheFPos();
                geom->ComputeBoundingBox();
                _entData[geom->GetID()].bbox.Update(it);
            }
        }
        it = NextSelectedEntity(it);
    }
}

void EntityEditor::CopyGlobalSelection() {
    // check if global selection is equal to editor selection
    W::Entity *self = FirstSelectedEntity(), *other = W::world.FirstSelectedEntity();
    while(self == other && self) {
        self = NextSelectedEntity(self);
        other = W::world.NextSelectedEntity(other);
    }
    if(self == other) return; // nothing to do

    // mirror selection
    _modifyGlobalSelection = false;
    ClearSelection();
    W::Entity* ent = W::world.FirstSelectedEntity();
    while(ent) {
        SelectEntity_Add(ent);
        ent = W::world.NextSelectedEntity(ent);
    }
    _modifyGlobalSelection = true;
    UpdateGizmo();
}

M::Vector3 EntityEditor::GlobalCenterOfSelection() {
    // TODO: cache this
    int numSelected = 0;
    M::Vector3 center = M::Vector3::Zero;
    W::Entity* ent = _selected;
    while(ent) {
        center += ent->GetGlobalCenter();
        numSelected++;
        ent = _entData[ent->GetID()].nextSelected;
    }
    center /= numSelected;
    return center;
}

W::Entity* EntityEditor::FirstSelectedEntity() {
    return _selected;
}

W::Entity* EntityEditor::NextSelectedEntity(const W::Entity* ent) {
    COM_assert(ent);
    COM_assert(IsSelected(ent));
    return _entData[ent->GetID()].nextSelected;
}

} // namespace NB