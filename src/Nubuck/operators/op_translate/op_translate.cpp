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

    _gizmo = nubuck().global_transform_gizmo();
}

void Translate::Register(Invoker& invoker) {
    /*
    no need for explicit invokation
    QAction* action = nubuck().scene_menu()->addAction("Translate");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    */

    if(!nubuck().first_selected_entity()) nubuck().hide_transform_gizmo(_gizmo);
}

bool Translate::Invoke() {
    nubuck().set_operator_name("Translate");
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

    if(W::editMode_t::OBJECTS == editMode) return g_nubuck.global_center_of_selection();

    if(W::editMode_t::VERTICES == editMode) {
        nb::geometry geom = nubuck().first_selected_geometry();
        std::vector<leda::node> verts = geom->GetVertexSelection();
        M::Vector3 pos = M::Vector3::Zero;
        M::Matrix4 objToWorld = geom->GetObjectToWorldMatrix();
        for(unsigned i = 0; i < verts.size(); ++i) {
            pos += M::Transform(objToWorld, ToVector(nubuck().poly_mesh(geom).position_of(verts[i])));
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
    nb::entity ent = nubuck().first_selected_entity();
    if(ent) {
        if(W::editMode_t::OBJECTS == _editMode) {
            isValidObjSelection = true;
        } else {
            assert(W::editMode_t::VERTICES == _editMode);
            if(nb::EntityType::GEOMETRY == nubuck().type_of(ent)) {
                nb::geometry geom = nubuck().to_geometry(ent);
                isValidVertSelection = !geom->GetVertexSelection().empty();
            }
        }
    }
    if(isValidObjSelection || isValidVertSelection) {
        nubuck().set_transform_gizmo_position(_gizmo, FindCursorPosition());
        nubuck().show_transform_gizmo(_gizmo);
    }
    else {
        nubuck().hide_transform_gizmo(_gizmo);
    }
}

bool Translate::DoPicking(const MouseEvent& event) {
	if(MouseEvent::BUTTON_RIGHT != event.button) return false;

    M::Ray ray = W::world.PickingRay(event.coords);

    if(W::editMode_t::OBJECTS == _editMode) {
        nb::entity ent = NULL;
        if(W::world.TraceEntity(ray, &ent)) {
            if(MouseEvent::MODIFIER_SHIFT & event.mods) nubuck().select(Nubuck::SELECT_MODE_ADD, ent);
            else nubuck().select(Nubuck::SELECT_MODE_NEW, ent);
            return true;
        }
    }

    if(W::editMode_t::VERTICES == _editMode && nubuck().first_selected_geometry()) {
        W::ENT_Geometry* geom = nubuck().first_selected_geometry();
        std::vector<W::ENT_Geometry::VertexHit> hits;
        if(geom->TraceVertices(ray, 0.2f, hits)) {
            // find nearest hit
            unsigned nidx = 0;
            for(unsigned i = 1; i < hits.size(); ++i) {
                if(hits[nidx].dist > hits[i].dist)
                    nidx = i;
            }

            Nubuck::SelectMode selectMode = Nubuck::SELECT_MODE_ADD;
            if(0 == (MouseEvent::MODIFIER_SHIFT & event.mods)) selectMode = Nubuck::SELECT_MODE_NEW;
            nubuck().select_vertex(selectMode, geom, hits[nidx].vert);
            nubuck().set_transform_gizmo_position(_gizmo, FindCursorPosition());

            W::SetColorsFromVertexSelection(*geom);
        }
    }

    return true;

}

void Translate::OnBeginDragging() {
    assert(nubuck().first_selected_entity());

    unsigned selectionSize = 0;
    nb::entity ent = nubuck().first_selected_entity();
    while(ent) {
        selectionSize++;
        ent = nubuck().next_selected_entity(ent);
    }

    _center = M::Vector3::Zero;

    // save entity positions
    if(W::editMode_t::OBJECTS == _editMode) {
        _oldEntityPos.resize(selectionSize);
        unsigned i = 0;
        nb::entity ent = nubuck().first_selected_entity();
        while(ent) {
            _oldEntityPos[i] = nubuck().position(ent);
            _center += _oldEntityPos[i];
            i++;
            ent = nubuck().next_selected_entity(ent);
        }
        _center /= _oldEntityPos.size();
    }

    // save vertex positions
    nb::geometry geom = nubuck().first_selected_geometry();
    if(geom) {
        const leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);
        _oldVertPos.init(mesh);
        leda::node v;
        forall_nodes(v, mesh) {
            const M::Vector3 pos = ToVector(mesh.position_of(v));
            _oldVertPos[v] = pos;
        }

        if(W::editMode_t::VERTICES == _editMode) {
            std::vector<leda::node> verts = geom->GetVertexSelection();
            for(unsigned i = 0; i < verts.size(); ++i)
                _center += _oldVertPos[verts[i]];
            _center /= verts.size();

            std::cout << "saving vert positions! " << verts.size() << std::endl;
        }
    }
}

void Translate::OnDragging(const Nubuck::transform_gizmo_mouse_info& info) {
    Nubuck::TransformGizmoMode::Enum mode = nubuck().transform_gizmo_mode(_gizmo);

    if(Nubuck::TransformGizmoMode::TRANSLATE == mode) {
        if(W::editMode_t::OBJECTS == _editMode) {
            unsigned i = 0;
            nb::entity ent = nubuck().first_selected_entity();
            while(ent) {
                M::Vector3 pos = _oldEntityPos[i];
                pos.vec[info.axis] = _oldEntityPos[i].vec[info.axis] + info.value;
                nubuck().set_position(ent, pos);
                i++;
                ent = nubuck().next_selected_entity(ent);
            }
        }

        if(W::editMode_t::VERTICES == _editMode) {
            nb::geometry geom = nubuck().first_selected_geometry();
            leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);

            std::vector<leda::node> verts = geom->GetVertexSelection();
            assert(!verts.empty());

            for(unsigned i = 0; i < verts.size(); ++i) {
                const leda::node v = verts[i];
                M::Vector3 pos = _oldVertPos[v];
                pos.vec[info.axis] = _oldVertPos[v].vec[info.axis] + info.value;
                mesh.set_position(v, ToRatPoint(pos));
            }
        }
    }
    if(Nubuck::TransformGizmoMode::SCALE == mode) {
        if(W::editMode_t::OBJECTS == _editMode) {
            // TODO: this is broken, does not work for multiple selected geometry objects
            nb::geometry geom = nubuck().first_selected_geometry();
            if(geom) {
                leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);
                leda::node v;
                forall_nodes(v, mesh) {
                    M::Vector3 pos = _oldVertPos[v];
                    pos.vec[info.axis] *= info.value;
                    mesh.set_position(v, ToRatPoint(pos));
                }
            }
        }

        if(W::editMode_t::VERTICES == _editMode) {
            nb::geometry geom = nubuck().first_selected_geometry();
            leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);

            std::vector<leda::node> verts = geom->GetVertexSelection();
            assert(!verts.empty());

            for(unsigned i = 0; i < verts.size(); ++i) {
                leda::node v = verts[i];
                M::Vector3 pos = _oldVertPos[v] - _center;
                pos.vec[info.axis] *= info.value;
                pos += _center;
                mesh.set_position(v, ToRatPoint(pos));
            }
        }
    }
}

void Translate::OnEditModeChanged(const W::editMode_t::Enum mode) {
    // stop dragging by resetting mode
    Nubuck::TransformGizmoMode::Enum tfMode = nubuck().transform_gizmo_mode(_gizmo);
    nubuck().set_transform_gizmo_mode(_gizmo, tfMode);

    _editMode = mode;
    UpdateCursor();

    // set colors of first selected geometry
    nb::entity ent = nubuck().first_selected_entity();
    if(ent && nb::EntityType::GEOMETRY == nubuck().type_of(ent)) {
        nb::geometry geom = nubuck().to_geometry(ent);
        leda::nb::RatPolyMesh& mesh = nubuck().poly_mesh(geom);

        if(W::editMode_t::OBJECTS == _editMode) {
            // restore old vertex colors
            leda::node v;
            forall_nodes(v, mesh) {
                mesh.set_color(v, _oldVertCol[v]);
            }
        } else {
            assert(W::editMode_t::VERTICES == _editMode);

            // save old vertex colors
            _oldVertCol.init(mesh);
            leda::node v;
            forall_nodes(v, mesh) {
                _oldVertCol[v] = mesh.color_of(v);
            }

            W::SetColorsFromVertexSelection(*geom);
        }
    }
}

bool Translate::OnMouse(const MouseEvent& event) {
    _editMode = W::world.GetEditMode().GetMode();

    Nubuck::transform_gizmo_mouse_info mouseInfo;
    if(nubuck().transform_gizmo_handle_mouse_event(_gizmo, event, mouseInfo)) {
        if(Nubuck::transform_gizmo_action::BEGIN_DRAGGING == mouseInfo.action) {
            OnBeginDragging();
        }
        if(Nubuck::transform_gizmo_action::DRAGGING == mouseInfo.action) {
            OnDragging(mouseInfo);
        }
        return true;
    } else { // event not handled by gizmo
        if(MouseEvent::MOUSE_DOWN == event.type) return DoPicking(event);
    }

    return false;
}

bool Translate::OnKey(const KeyEvent& event) {
    // scancodes for number row of generic usb keyboard
    static const unsigned numrow[3] = { 11, 2, 3 };

    if(numrow[1] == event.nativeScanCode) {
        nubuck().set_transform_gizmo_mode(_gizmo, Nubuck::TransformGizmoMode::TRANSLATE);
    }
    if(numrow[2] == event.nativeScanCode) {
        nubuck().set_transform_gizmo_mode(_gizmo, Nubuck::TransformGizmoMode::SCALE);
    }

    // do not become active when only mode changes
    return false;
}

} // namespace OP