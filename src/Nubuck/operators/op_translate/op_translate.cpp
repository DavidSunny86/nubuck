#include <Nubuck\polymesh.h>
#include <Nubuck\math\intersections.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\cone\cone.h>
#include "op_translate.h"

namespace OP {

struct AxisMesh {
    std::vector<R::Mesh::Vertex>    vertices;
    std::vector<R::Mesh::Index>     indices;

    AxisMesh(
        float size, 
        int subdiv, 
        float spacing,
        const R::Color& colX, 
        const R::Color& colY, 
        const R::Color& colZ) 
    {
        unsigned idxCnt = 0;
        R::Color colors[] = { colX, colY, colZ };

        unsigned N = (1 << subdiv);
        float f = size / N;
        for(int i = 0; i < 3; ++i) {
            M::Vector3 d = M::Vector3::Zero;
            d.vec[i] = 1.0f;

            for(int j = 0; j < N; ++j) {
                R::Mesh::Vertex vert;
                vert.position = f * d * j;
                vert.color = colors[i];
                vertices.push_back(vert);
                indices.push_back(idxCnt++);

                M::Vector3 p = f * d * (j + 1) - spacing * f * d;
                vert.position = p;
                vert.color = colors[i];
                vertices.push_back(vert);
                indices.push_back(idxCnt++);
            }
        }
    }

    R::Mesh::Desc GetDesc() {
        R::Mesh::Desc desc;
        desc.vertices = &vertices[0];
        desc.numVertices = vertices.size();
        desc.indices = &indices[0];
        desc.numIndices = indices.size();
        desc.primType = GL_LINES;
        return desc;
    }
};

Translate::Translate() : _dragging(false) {
	AddEventHandler(EV::def_SelectionChanged, this, &Translate::Event_SelectionChanged);

    _cursorPos = M::Vector3::Zero;
    _hidden = true;

    AxisMesh axisDesc(1.0f, 4, 0.4f, R::Color::Red, R::Color::Blue, R::Color::Green);
    _axisMesh = R::meshMgr.Create(axisDesc.GetDesc());
    _axisTFMesh = R::meshMgr.Create(_axisMesh);

    R::Cone arrowHead(0.1f, 0.5f, 20, R::Color::Red);
    R::meshPtr_t meshPtr = _arrowHeadMeshes[0] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Red).GetDesc());
    _arrowHeadTFMeshes[0] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[0] = M::Mat4::Translate(1.0f, 0.0f, 0.0f) * M::Mat4::RotateY( 90.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[0]).SetTransform(_arrowHeadTF[0]);

    meshPtr = _arrowHeadMeshes[1] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Blue).GetDesc());
    _arrowHeadTFMeshes[1] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[1] = M::Mat4::Translate(0.0f, 1.0f, 0.0f) * M::Mat4::RotateX(-90.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[1]).SetTransform(_arrowHeadTF[1]);

    meshPtr = _arrowHeadMeshes[2] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Green).GetDesc());
    _arrowHeadTFMeshes[2] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[2] = M::Mat4::Translate(0.0f, 0.0f, 1.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[2]).SetTransform(_arrowHeadTF[2]);
}

void Translate::BuildBBoxes() {
    const float l = 1.2f;
    const float w = 0.4f;
    _bboxes[X] = M::Box::FromCenterSize(M::Vector3(0.5f, 0.0f, 0.0f), M::Vector3(l, w, w));
    _bboxes[Y] = M::Box::FromCenterSize(M::Vector3(0.0f, 0.5f, 0.0f), M::Vector3(w, l, w));
    _bboxes[Z] = M::Box::FromCenterSize(M::Vector3(0.0f, 0.0f, 0.5f), M::Vector3(w, w, l));
}

void Translate::HideCursor() {
    _hidden = true;
}

void Translate::ShowCursor() {
    _hidden = false;
}

static void SetCenterPosition(M::Box& box, const M::Vector3& center) {
    const M::Vector3 oldCenter = 0.5f * (box.max - box.min) + box.min;
    const M::Vector3 d = (center - oldCenter);
    box.min += d;
    box.max += d;
}

void Translate::SetRenderPosition(const M::Vector3& pos) {
    M::Matrix4 T = M::Mat4::Translate(pos);
    R::meshMgr.GetMesh(_axisTFMesh).SetTransform(T);
    for(int i = 0; i < DIM; ++i) {
        R::meshMgr.GetMesh(_arrowHeadTFMeshes[i]).SetTransform(T * _arrowHeadTF[i]);
    }
    for(unsigned i = 0; i < DIM; ++i) {
        SetCenterPosition(_bboxes[i], pos);
        const float l = 1.2f;
        const float w = 0.2f;
        _bboxes[X] = M::Box::FromCenterSize(pos + M::Vector3(0.5f, 0.0f, 0.0f), M::Vector3(l, w, w));
        _bboxes[Y] = M::Box::FromCenterSize(pos + M::Vector3(0.0f, 0.5f, 0.0f), M::Vector3(w, l, w));
        _bboxes[Z] = M::Box::FromCenterSize(pos + M::Vector3(0.0f, 0.0f, 0.5f), M::Vector3(w, w, l));
    }
}

/*
====================
AlignWithCamera
    moves vector v at constant distance to camera.
    note: worldToEye = modelview matrix
====================
*/
static M::Vector3 AlignWithCamera(const M::Matrix4& worldToEye, const M::Vector3& v) {
    M::Matrix4 eyeToWorld;
    bool r = true;
    r = M::TryInvert(worldToEye, eyeToWorld);
    COM_assert(r);
    M::Vector3 eye = M::Transform(eyeToWorld, M::Vector3::Zero); // eye pos in world space
    const float c = 10.0f;
    M::Vector3 d = v - eye;
    M::Matrix4 M = M::Mat4::Translate(-(d.Length() - c) * M::Normalize(d));
    return M::Transform(M, v);
}

void Translate::SetCursorPosition(const M::Vector3& pos) {
    _cursorPos = pos;
    M::Vector3 renderPos = AlignWithCamera(W::world.GetModelView(), _cursorPos);
    SetRenderPosition(renderPos);
}

void Translate::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    /*
    no need for explicit invokation
    QAction* action = _nb.ui->GetSceneMenu()->addAction("Translate");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    */

    if(!W::world.GetSelection()->GetList().empty()) HideCursor();
}

static M::Vector3 Axis(int i) {
    if(0 == i) return M::Vector3(1.0f, 0.0f, 0.0f);
    if(1 == i) return M::Vector3(0.0f, 1.0f, 0.0f);
    if(2 == i) return M::Vector3(0.0f, 0.0f, 1.0f);
    assert(false && "Axis(int i): parameter out of range");
    return M::Vector3::Zero;
}

void Translate::Invoke() {
    _nb.ui->SetOperatorName("Translate");
}

void Translate::GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
    if(_hidden) return;

    R::MeshJob meshJob;

    meshJob.fx = "UnlitThickLines";
    meshJob.layer = 1;
    meshJob.material = R::Material::White;
    meshJob.primType = 0;
    meshJob.tfmesh = _axisTFMesh;
    meshJobs.push_back(meshJob);

    meshJob.fx = "LitDirectional";
    meshJob.layer = 1;
    meshJob.material = R::Material::White;
    meshJob.primType = 0;
    for(int i = 0; i < 3; ++i) {
        meshJob.tfmesh = _arrowHeadTFMeshes[i];
        meshJobs.push_back(meshJob);
    }
}

void Translate::OnGeometrySelected() {
    UpdateCursor();
}

void Translate::OnCameraChanged() {
    SetCursorPosition(_cursorPos); // updates renderpos
}

bool Translate::TraceCursor(const M::Ray& ray, int& axis, M::IS::Info* inf) {
    for(int i = 0; i < DIM; ++i) {
        if(M::IS::Intersects(ray, _bboxes[i], inf)) {
            axis = i;
            return true;
        }
    }
    return false;
}

// returns z axis of eye space in world space
static M::Vector3 EyeZ(const M::Matrix4& modelView) {
    M::Matrix3 M = M::RotationOf(modelView);
    float det = M::Det(M);
    if(M::AlmostEqual(0.0f, det)) common.printf("WARNING - modelview matrix is singular\n");
    M::Matrix3 invM = M::Inverse(M, det);
    return M::Transform(invM, M::Vector3(0.0f, 0.0f, 1.0f));
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

static M::Vector3 FindCursorPosition(ISelection* sel) {
    const W::editMode_t::Enum editMode = W::world.GetEditMode().GetMode();

    if(W::editMode_t::OBJECTS == editMode) return sel->GetGlobalCenter();
    
    if(W::editMode_t::VERTICES == editMode) {
        W::ENT_Geometry* geom = (W::ENT_Geometry*)sel->GetList().front();
        std::vector<leda::node> verts = geom->GetVertexSelection();
        M::Vector3 pos = M::Vector3::Zero;
        for(unsigned i = 0; i < verts.size(); ++i) {
            pos += geom->Transform(ToVector(geom->GetRatPolyMesh().position_of(verts[i])));
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
    ISelection* sel = W::world.GetSelection();
	if(sel->GetList().empty()) HideCursor();
    else {
        SetCursorPosition(FindCursorPosition(sel));
        ShowCursor();
    }
}

bool Translate::OnMouseDown(const MouseEvent& event) {
	if(MouseEvent::BUTTON_RIGHT != event.button) return false;

    if(_dragging) return false;

    M::Ray ray = W::world.PickingRay(event.coords);

    int         axis;
    M::IS::Info inf;

    _editMode = W::world.GetEditMode().GetMode();

    if(!W::world.GetSelection()->GetList().empty() && TraceCursor(ray, axis, &inf)) {
        M::Vector3 eyeZ = EyeZ(W::world.GetModelView());
        M::Vector3 vAxis = Axis(axis);
        _dragAxis = axis;
        _dragPlane = M::Plane::FromPointSpan(_cursorPos, M::Cross(eyeZ, vAxis), vAxis);
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        _dragOrig = inf.where;

        const std::vector<IGeometry*>& geomList = W::world.GetSelection()->GetList();

        if(W::editMode_t::OBJECTS == _editMode) {
            _oldGeomPos.resize(geomList.size());
            for(unsigned i = 0; i < _oldGeomPos.size(); ++i) 
                _oldGeomPos[i] = ((const W::ENT_Geometry*)geomList[i])->Transform(M::Vector3::Zero);
        }

        if(W::editMode_t::VERTICES == _editMode) {
            const W::ENT_Geometry* geom = (const W::ENT_Geometry*)geomList[0];
            const leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();
            _oldVertPos.init(mesh);
            leda::node v;
            forall_nodes(v, mesh) {
                const M::Vector3 pos = ToVector(mesh.position_of(v));
                _oldVertPos[v] = pos;
            }
        }

        _oldCursorPos = _cursorPos;
        _dragging = true;
    } else {
        if(W::editMode_t::OBJECTS == _editMode) {
            W::ENT_Geometry* geom;
            if(W::world.Trace(ray, &geom)) {
                if(MouseEvent::MODIFIER_SHIFT & event.mods) W::world.GetSelection()->Add(geom);
                else W::world.GetSelection()->Set(geom);
                return true;
            }
        }

        if(W::editMode_t::VERTICES == _editMode && !W::world.GetSelection()->GetList().empty()) {
            W::ENT_Geometry* geom = (W::ENT_Geometry*)W::world.GetSelection()->GetList().front();
            std::vector<leda::node> verts;
            if(geom->TraceVertices(ray, 0.2f, verts)) {
                ISelection::SelectMode selectMode = ISelection::SELECT_ADD;
                if(0 == (MouseEvent::MODIFIER_SHIFT & event.mods)) selectMode = ISelection::SELECT_NEW;
                W::world.GetSelection()->SelectVertex(selectMode, geom, verts[0]);
                SetCursorPosition(FindCursorPosition(W::world.GetSelection()));
            }
        }
    }
    return false;
}

bool Translate::OnMouseUp(const MouseEvent&) {
    if(_dragging) {
        _dragging = false;
        return true;
    }
    return false;
}

bool Translate::OnMouseMove(const MouseEvent& event) {
    if(_dragging) {
        M::Ray ray = W::world.PickingRay(event.coords);
        M::IS::Info inf;
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        M::Vector3 p = inf.where;
        M::Vector3 pos = _cursorPos;
        pos.vec[_dragAxis] = _oldCursorPos.vec[_dragAxis] + (p - _dragOrig).vec[_dragAxis];
        SetCursorPosition(pos);

        std::vector<IGeometry*>& geomList = W::world.GetSelection()->GetList();

        if(W::editMode_t::OBJECTS == _editMode) {
            for(unsigned i = 0; i < geomList.size(); ++i) {
                IGeometry* geom = geomList[i];
                M::Vector3 pos = _oldGeomPos[i];
                pos.vec[_dragAxis] = _oldGeomPos[i].vec[_dragAxis] + (p - _dragOrig).vec[_dragAxis];
                geom->SetPosition(pos.x, pos.y, pos.z);
            }
        }

        if(W::editMode_t::VERTICES == _editMode) {
            W::ENT_Geometry* geom = (W::ENT_Geometry*)geomList[0];
            leda::nb::RatPolyMesh& mesh = geom->GetRatPolyMesh();

            std::vector<leda::node> verts = geom->GetVertexSelection();
            assert(!verts.empty());

            for(unsigned i = 0; i < verts.size(); ++i) {
                const leda::node v = verts[i];
                M::Vector3 pos = _oldVertPos[v];
                pos.vec[_dragAxis] = _oldVertPos[v].vec[_dragAxis] + (p - _dragOrig).vec[_dragAxis];
                mesh.set_position(v, ToRatPoint(pos));
            }
        }
        return true;
    }
    return false;
}

void Translate::OnEditModeChanged(const W::editMode_t::Enum mode) {
    _dragging = false;
    UpdateCursor();
}

bool Translate::OnMouse(const MouseEvent& event) {
	switch(event.type) {
	case MouseEvent::MOUSE_DOWN:  return OnMouseDown(event);
	case MouseEvent::MOUSE_UP:    return OnMouseUp(event);
	case MouseEvent::MOUSE_MOVE:  return OnMouseMove(event);
	}
    return false;
}

} // namespace OP