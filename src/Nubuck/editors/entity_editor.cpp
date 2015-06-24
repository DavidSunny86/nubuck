#include <Nubuck\math_conv.h>
#include <Nubuck\editors\entity_editor.h>
#include <Nubuck\face_vertex_mesh.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>

namespace NB {

/*
==================================================
TranslateImpl implementation {
==================================================
*/

EntityEditor::TranslateImpl::TranslateImpl(EntityEditor* entityEditor) : entityEditor(entityEditor) { }

void EntityEditor::TranslateImpl::OnBeginDragging() {
    W::Entity* ent = entityEditor->FirstSelectedEntity();
    while(ent) {
        entityEditor->_entData[ent->GetID()].oldPos = ent->GetPosition();
        ent = entityEditor->NextSelectedEntity(ent);
    }
}

void EntityEditor::TranslateImpl::OnDragging() {
    int dragAxis = entityEditor->GetDragAxis();
    float translation = entityEditor->GetTranslation();

    W::Entity* ent = entityEditor->FirstSelectedEntity();
    while(ent) {
        EntityData& data = entityEditor->_entData[ent->GetID()];

        M::Vector3 pos = data.oldPos;
        pos.vec[dragAxis] += translation;
        ent->SetPosition(pos);

        // move bbox
        COM_assert(data.bbox.geom);
        data.bbox.geom->SetPosition(pos);

        ent = entityEditor->NextSelectedEntity(ent);
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

/*
NOTE: scaling always sets vertex positions and never sets scale of object-to-world matrix.
this is odd and might get changed in the future.
*/

EntityEditor::ScaleImpl::ScaleImpl(EntityEditor* entityEditor)
    : entityEditor(entityEditor)
    , lastScale(1.0f, 1.0f, 1.0f)
    , scale(1.0f, 1.0f, 1.0f)
{ }

void EntityEditor::ScaleImpl::OnEnter() {
    lastScale = scale = M::Vector3(1.0f, 1.0f, 1.0f);
}

void EntityEditor::ScaleImpl::OnBeginDragging() {
    W::ENT_Geometry* geom = NB::FirstSelectedMesh(); // naming confusion, oh boy
    while(geom) {
        const leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        EntityData& entData = entityEditor->GetEntityData(geom);
        entData.oldVertPosF.init(mesh);

        leda::node v;
        forall_nodes(v, mesh) {
            entData.oldVertPosF[v] = ToVector(mesh.position_of(v));
        }

        geom = NB::NextSelectedMesh(geom);
    }
}

void EntityEditor::ScaleImpl::OnDragging() {
    int dragAxis = entityEditor->GetDragAxis();
    float scale = entityEditor->GetScale();

    // update total scaling vector
    this->scale.vec[dragAxis] = lastScale.vec[dragAxis] * scale;

    M::Vector3 center_ws = entityEditor->GlobalCenterOfSelection();

    W::ENT_Geometry* geom = NB::FirstSelectedMesh();
    while(geom) {
        EntityData& entData = entityEditor->GetEntityData(geom);
        leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
        leda::node v;
        forall_nodes(v, mesh) {
            M::Vector3 pos = entData.oldVertPosF[v];

            // scale around center

            M::Matrix4 worldToObject;
            bool isRegular = M::TryInvert(geom->GetObjectToWorldMatrix(), worldToObject);
            if(!isRegular) {
                common.printf("WARNING - object-to-world matrix of geometry with id=%d is not invertable\n", geom->GetID());
            }

            M::Vector3 center_os = M::Transform(worldToObject, center_ws);

            pos -= center_os;
            pos.vec[dragAxis] *= scale;
            pos += center_os;

            mesh.set_position(v, ToRatPoint(pos));
        }
        geom = NB::NextSelectedMesh(geom);
    }
}

void EntityEditor::ScaleImpl::OnEndDragging() {
    lastScale = scale;
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
    geom->SetEdgeTint(R::Color::White);

    leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

    // TODO: simplify this
    MakeCube(mesh);
    int vidx = 0;
    leda::node v;
    forall_nodes(v, mesh) {
        verts[vidx++] = v;
    }
}

void EntityEditor::BBox::Update(W::Entity* ent) {
    const M::Box mbox = ent->GetBoundingBox();

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
}

void EntityEditor::SelectEntity_Add(W::Entity* ent) {
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

        // save current position of selected entities
        // TODO: merge with above loop
        W::Entity* ent = _selected;
        while(ent) {
            EntityData& data = _entData[ent->GetID()];
            data.oldPos = ent->GetPosition();
            ent = data.nextSelected;
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
                _lastAction = Action_SelectEntity_Add;
                if(!(EV::MouseEvent::MODIFIER_SHIFT & event.mods)) {
                    ClearSelection();
                    _lastAction = Action_SelectEntity;
                }
                SelectEntity_Add(ent);
                UpdateGizmo();
            }
            return true;
        }
    }
    return false;
}

EntityEditor::EntityData& EntityEditor::GetEntityData(W::Entity* ent) {
    return _entData[ent->GetID()];
}

void EntityEditor::OnBeginDragging() {
    _curImpl->OnBeginDragging();
    _lastAction = Action_BeginDragging;
}

void EntityEditor::OnDragging() {
    _curImpl->OnDragging();
    _lastAction = Action_Dragging;
}

void EntityEditor::OnEndDragging() {
    _curImpl->OnEndDragging();
    _lastAction = Action_EndDragging;
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
                int numSelected = 0;
                ent = W::world.FirstEntity();
                while(ent) {
                    if(ent->IsSolid()) {
                        SelectEntity_Add(ent);
                        numSelected++;
                    }
                    ent = W::world.NextEntity(ent);
                }
                std::cout << "EntityEditor: select all, " << numSelected << " selected." << std::endl;
            }

            _lastAction = Action_SelectEntity;

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
    , _lastAction(Action_None)
{
    _scaleImpl = GEN::MakePtr(new ScaleImpl(this));
    _impl[0] = GEN::MakePtr(new TranslateImpl(this));
    _impl[1] = _scaleImpl;
    _curImpl = _impl[0].Raw();
}

void EntityEditor::SetAllowedModeFlags(int flags) {
    _allowedModeFlags = flags;
}

void EntityEditor::SetMode(int mode) {
    if(mode == _mode) return;

    if(!(_allowedModeFlags & 1 << mode)) {
        COM_printf("WARNING - EntityEditor::SetMode(): required mode not allowed.\n");
        return;
    }

    _mode = mode;
    SetGizmoTransformMode(NB::TransformGizmoMode(_mode));
    _curImpl = _impl[_mode].Raw();
    _curImpl->OnEnter();
}

// clears selection
void EntityEditor::Open() {
    ClearSelection();
    UpdateGizmo();
}

void EntityEditor::Close() {
    SetGizmoVisibility(false);
}

void EntityEditor::SetTranslationVector(const M::Vector3& v) {
}

// update
void EntityEditor::UpdateBoundingBoxes() {
    W::Entity* it = FirstSelectedEntity();
    while(it) {
        if(W::EntityType::ENT_GEOMETRY == it->GetType()) {
            W::ENT_Geometry* geom = static_cast<W::ENT_Geometry*>(it);
            M::Vector3 translation = geom->GetPosition() - _entData[geom->GetID()].bbox.geom->GetPosition();
            bool moved =
                0.0f != translation.x ||
                0.0f != translation.y ||
                0.0f != translation.z;
            if(geom->IsDirty() || moved) {
                geom->CacheFPos();
                _entData[geom->GetID()].bbox.Update(it);
            }
        }
        it = NextSelectedEntity(it);
    }
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

const W::Entity* EntityEditor::FirstSelectedEntity() const {
    return _selected;
}

W::Entity* EntityEditor::NextSelectedEntity(const W::Entity* ent) {
    COM_assert(ent);
    COM_assert(IsSelected(ent));
    return _entData[ent->GetID()].nextSelected;
}

EntityEditor::Mode EntityEditor::GetMode() const {
    return Mode(_mode);
}

EntityEditor::Action EntityEditor::GetAction() const {
    return Action(_lastAction);
}

M::Vector3 EntityEditor::GetTranslationVector() const {
    const W::Entity* ent = FirstSelectedEntity();
    if(ent) {
        // the translation vector is the same for all selected entities
        return ent->GetPosition() - _entData[ent->GetID()].initialPos;
    }
    return M::Vector3::Zero;
}

M::Vector3 EntityEditor::GetScalingVector() const {
    return _scaleImpl->scale;
}

void EntityEditor::CopyGlobalSelection() {
    // test if global selection differs from editor selection
    W::Entity* self = FirstSelectedEntity();
    W::Entity* other = NB::FirstSelectedEntity();
    while(self && self == other) {
        self = NextSelectedEntity(self);
        other = NB::NextSelectedEntity(other);
    }
    if(self == other) return; // nothing to do

    ClearSelection();
    W::Entity* ent = NB::FirstSelectedEntity();
    while(ent) {
        SelectEntity_Add(ent);
        ent = NB::NextSelectedEntity(ent);
    }
}

} // namespace NB