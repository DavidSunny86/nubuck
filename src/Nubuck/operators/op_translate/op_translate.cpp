#include <maxint.h>

#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include <Nubuck\math\intersections.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\cone\cone.h>
#include <renderer\mesh\sphere\sphere.h>
#include <nubuck_private.h>
#include "op_translate.h"

namespace OP {

Translate::Translate() : _gizmo(0) {
	AddEventHandler(ev_w_selectionChanged, this, &Translate::Event_SelectionChanged);

    _gizmo = NB::GlobalTransformGizmo();
}

void Translate::Register(Invoker& invoker) {
    /*
    no need for explicit invokation
    QAction* action = nubuck().scene_menu()->addAction("Translate");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    */

    if(!NB::FirstSelectedEntity()) NB::HideTransformGizmo(_gizmo);
}

bool Translate::Invoke() {
    NB::SetOperatorName("Translate");
    _editMode = W::world.GetEditMode().GetMode();
    OnGeometrySelected();
    return true;
}

void Translate::OnGeometrySelected() {
    UpdateCursor();
}

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

static M::Vector3 FindCursorPosition() {
    const W::editMode_t::Enum editMode = W::world.GetEditMode().GetMode();

    if(W::editMode_t::OBJECTS == editMode) return NB::GlobalCenterOfSelection();

    if(W::editMode_t::VERTICES == editMode) {
        NB::Mesh mesh = NB::FirstSelectedMesh();
        std::vector<leda::node> verts = mesh->GetVertexSelection();
        M::Vector3 pos = M::Vector3::Zero;
        M::Matrix4 objToWorld = mesh->GetObjectToWorldMatrix();
        for(unsigned i = 0; i < verts.size(); ++i) {
            pos += M::Transform(objToWorld, ToVector(NB::GetGraph(mesh).position_of(verts[i])));
        }
        float d = 1.0f / verts.size();
        return d * pos;
    }

    assert(false && "unreachable");
    return M::Vector3::Zero;
}

/*
====================
Translate::UpdateCursor
    updates visibility and position
====================
*/
void Translate::UpdateCursor() {
    bool isValidObjSelection = false, isValidVertSelection = false;
    NB::Entity ent = NB::FirstSelectedEntity();
    if(ent) {
        if(W::editMode_t::OBJECTS == _editMode) {
            isValidObjSelection = true;
        } else {
            assert(W::editMode_t::VERTICES == _editMode);
            if(NB::ET_GEOMETRY == NB::GetType(ent)) {
                NB::Mesh mesh = NB::CastToMesh(ent);
                isValidVertSelection = !mesh->GetVertexSelection().empty();
            }
        }
    }
    if(isValidObjSelection || isValidVertSelection) {
        NB::SetTransformGizmoPosition(_gizmo, FindCursorPosition());
        NB::ShowTransformGizmo(_gizmo);
    }
    else {
        NB::HideTransformGizmo(_gizmo);
    }
}

bool Translate::DoPicking(const EV::MouseEvent& event) {
	if(EV::MouseEvent::BUTTON_RIGHT != event.button) return false;

    M::Ray ray = W::world.PickingRay(M::Vector2(event.x, event.y));

    if(W::editMode_t::OBJECTS == _editMode) {
        NB::Entity ent = NULL;
        if(W::world.TraceEntity(ray, &ent)) {
            if(EV::MouseEvent::MODIFIER_SHIFT & event.mods) NB::SelectEntity(NB::SM_ADD, ent);
            else NB::SelectEntity(NB::SM_NEW, ent);
            return true;
        }
    }

    if(W::editMode_t::VERTICES == _editMode && NB::FirstSelectedMesh()) {
        NB::Mesh mesh = NB::FirstSelectedMesh();
        std::vector<W::ENT_Geometry::VertexHit> hits;
        if(mesh->TraceVertices(ray, 0.2f, hits)) {
            // find nearest hit
            unsigned nidx = 0;
            for(unsigned i = 1; i < hits.size(); ++i) {
                if(hits[nidx].dist > hits[i].dist)
                    nidx = i;
            }

            NB::SelectMode selectMode = NB::SM_ADD;
            if(0 == (EV::MouseEvent::MODIFIER_SHIFT & event.mods)) selectMode = NB::SM_NEW;
            NB::SelectVertex(selectMode, mesh, hits[nidx].vert);
            NB::SetTransformGizmoPosition(_gizmo, FindCursorPosition());

            W::SetColorsFromVertexSelection(*mesh);
        }
    }

    return true;

}

void Translate::OnBeginDragging() {
    assert(NB::FirstSelectedEntity());

    unsigned selectionSize = 0;
    NB::Entity ent = NB::FirstSelectedEntity();
    while(ent) {
        selectionSize++;
        ent = NB::NextSelectedEntity(ent);
    }

    _center = M::Vector3::Zero;

    // save entity positions
    if(W::editMode_t::OBJECTS == _editMode) {
        _oldEntityPos.resize(selectionSize);
        unsigned i = 0;
        NB::Entity ent = NB::FirstSelectedEntity();
        while(ent) {
            _oldEntityPos[i] = NB::GetEntityPosition(ent);
            _center += _oldEntityPos[i];
            i++;
            ent = NB::NextSelectedEntity(ent);
        }
        _center /= _oldEntityPos.size();
    }

    // save vertex positions
    NB::Mesh mesh = NB::FirstSelectedMesh();
    if(mesh) {
        const leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);
        _oldVertPos.init(graph);
        leda::node v;
        forall_nodes(v, graph) {
            const M::Vector3 pos = ToVector(graph.position_of(v));
            _oldVertPos[v] = pos;
        }

        if(W::editMode_t::VERTICES == _editMode) {
            std::vector<leda::node> verts = mesh->GetVertexSelection();
            for(unsigned i = 0; i < verts.size(); ++i)
                _center += _oldVertPos[verts[i]];
            _center /= verts.size();

            std::cout << "saving vert positions! " << verts.size() << std::endl;
        }
    }
}

void Translate::OnDragging(const NB::TransformGizmoMouseInfo& info) {
    NB::TransformGizmoMode mode = NB::GetTransformGizmoMode(_gizmo);

    if(NB::TGM_TRANSLATE == mode) {
        if(W::editMode_t::OBJECTS == _editMode) {
            unsigned i = 0;
            NB::Entity ent = NB::FirstSelectedEntity();
            while(ent) {
                M::Vector3 pos = _oldEntityPos[i];
                pos.vec[info.axis] = _oldEntityPos[i].vec[info.axis] + info.value;
                NB::SetEntityPosition(ent, pos);
                i++;
                ent = NB::NextSelectedEntity(ent);
            }
        }

        if(W::editMode_t::VERTICES == _editMode) {
            NB::Mesh mesh = NB::FirstSelectedMesh();
            leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);

            std::vector<leda::node> verts = mesh->GetVertexSelection();
            assert(!verts.empty());

            for(unsigned i = 0; i < verts.size(); ++i) {
                const leda::node v = verts[i];
                M::Vector3 pos = _oldVertPos[v];
                pos.vec[info.axis] = _oldVertPos[v].vec[info.axis] + info.value;
                graph.set_position(v, ToRatPoint(pos));
            }
        }
    }
    if(NB::TGM_SCALE == mode) {
        if(W::editMode_t::OBJECTS == _editMode) {
            // TODO: this is broken, does not work for multiple selected geometry objects
            NB::Mesh mesh = NB::FirstSelectedMesh();
            if(mesh) {
                leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);
                leda::node v;
                forall_nodes(v, graph) {
                    M::Vector3 pos = _oldVertPos[v];
                    pos.vec[info.axis] *= info.value;
                    graph.set_position(v, ToRatPoint(pos));
                }
            }
        }

        if(W::editMode_t::VERTICES == _editMode) {
            NB::Mesh mesh = NB::FirstSelectedMesh();
            leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);

            std::vector<leda::node> verts = mesh->GetVertexSelection();
            assert(!verts.empty());

            for(unsigned i = 0; i < verts.size(); ++i) {
                leda::node v = verts[i];
                M::Vector3 pos = _oldVertPos[v] - _center;
                pos.vec[info.axis] *= info.value;
                pos += _center;
                graph.set_position(v, ToRatPoint(pos));
            }
        }
    }
}

void Translate::OnEditModeChanged(const W::editMode_t::Enum mode) {
    // stop dragging by resetting mode
    NB::TransformGizmoMode tfMode = NB::GetTransformGizmoMode(_gizmo);
    NB::SetTransformGizmoMode(_gizmo, tfMode);

    _editMode = mode;
    UpdateCursor();

    // set colors of first selected geometry
    NB::Entity ent = NB::FirstSelectedEntity();
    if(ent && NB::ET_GEOMETRY == NB::GetType(ent)) {
        NB::Mesh mesh = NB::CastToMesh(ent);
        leda::nb::RatPolyMesh& graph = NB::GetGraph(mesh);

        if(W::editMode_t::OBJECTS == _editMode) {
            // restore vertex, edge colors
            leda::node v;
            forall_nodes(v, graph) {
                graph.set_color(v, _oldVertCol[v]);
            }
            leda::edge e;
            forall_edges(e, graph) {
                graph.set_color(e, _oldEdgeCol[e]);
            }
        } else {
            assert(W::editMode_t::VERTICES == _editMode);

            // save current vertex, edge colors
            _oldVertCol.init(graph);
            leda::node v;
            forall_nodes(v, graph) {
                _oldVertCol[v] = graph.color_of(v);
            }
            _oldEdgeCol.init(graph);
            leda::edge e;
            forall_edges(e, graph) {
                _oldEdgeCol[e] = graph.color_of(e);
            }

            W::SetColorsFromVertexSelection(*mesh);
        }
    }
}

bool Translate::OnMouse(const EV::MouseEvent& event) {
    _editMode = W::world.GetEditMode().GetMode();

    NB::TransformGizmoMouseInfo mouseInfo;
    if(NB::TransformGizmoHandleMouseEvent(_gizmo, event, mouseInfo)) {
        if(NB::TGA_BEGIN_DRAGGING == mouseInfo.action) {
            OnBeginDragging();
        }
        if(NB::TGA_DRAGGING == mouseInfo.action) {
            OnDragging(mouseInfo);
        }
        return true;
    } else { // event not handled by gizmo
        if(EV::MouseEvent::MOUSE_DOWN == event.type) return DoPicking(event);
    }

    return false;
}

bool Translate::OnKey(const EV::KeyEvent& event) {
    // scancodes for number row of generic usb keyboard
    static const unsigned numrow[3] = { 11, 2, 3 };

    if(numrow[1] == event.nativeScanCode) {
        NB::SetTransformGizmoMode(_gizmo, NB::TGM_TRANSLATE);
    }
    if(numrow[2] == event.nativeScanCode) {
        NB::SetTransformGizmoMode(_gizmo, NB::TGM_SCALE);
    }

    // do not become active when only mode changes
    return false;
}

} // namespace OP