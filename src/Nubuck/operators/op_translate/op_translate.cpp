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
    _arrowHeadTF[0] = M::Mat4::Translate(1.0f, 0.0f, 0.0f) * M::Mat4::RotateY(-90.0f);
    R::meshMgr.GetMesh(_arrowHeadTFMeshes[0]).SetTransform(_arrowHeadTF[0]);

    meshPtr = _arrowHeadMeshes[1] = R::meshMgr.Create(R::Cone(0.1f, 0.5f, 20, R::Color::Blue).GetDesc());
    _arrowHeadTFMeshes[1] = R::meshMgr.Create(meshPtr);
    _arrowHeadTF[1] = M::Mat4::Translate(0.0f, 1.0f, 0.0f) * M::Mat4::RotateX( 90.0f);
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

void Translate::BuildCursor() {
}

void Translate::HideCursor() {
    _hidden = true;
}

void Translate::ShowCursor() {
    _hidden = false;
}

void Translate::AlignWithCamera() {
    M::Matrix4 eyeToWorld, worldToEye = W::world.GetModelView();
    bool r = true;
    r = M::TryInvert(worldToEye, eyeToWorld);
    assert(r);
    M::Vector3 eye = M::Transform(eyeToWorld, M::Vector3::Zero); // eye pos in world space
    const float c = 10.0f;
    M::Vector3 d = _cursorPos - eye;
    M::Matrix4 M = M::Mat4::Translate(-(d.Length() - c) * M::Normalize(d));
    W::ENT_Geometry* ent = NULL;
    SetPosition(M::Transform(M, _cursorPos));
}

void Translate::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    /*
    no need for explicit invokation
    QAction* action = _nb.ui->GetSceneMenu()->addAction("Translate");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
    */

    BuildCursor();

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
	if(W::world.GetSelection()->GetList().empty()) HideCursor();
    else {
        _cursorPos = W::world.GetSelection()->GetGlobalCenter();
        SetPosition(_cursorPos);

        AlignWithCamera();
        ShowCursor();
    }
}

void Translate::OnCameraChanged() {
    if(!W::world.GetSelection()->GetList().empty()) // ie. cursor is visible
        AlignWithCamera();
}

static float tmp;

bool Translate::OnMouseDown(const MouseEvent& event) {
	if(MouseEvent::BUTTON_RIGHT != event.button) return false;

    M::Ray ray = W::world.PickingRay(event.coords);
    if(!_dragging) {
        if(!W::world.GetSelection()->GetList().empty()) {
            M::Matrix3 M = M::RotationOf(W::world.GetModelView());
            float det = M::Det(M);
            if(M::AlmostEqual(0.0f, det)) printf("OMGOMGOMG\n");
            M::Matrix3 invM = M::Inverse(M, det);

            M::Vector3 eyeZ = M::Transform(invM, M::Vector3(0.0f, 0.0f, 1.0f)); // z axis of eye space in world space
            printf("eyeZ = %f %f %f\n", eyeZ.x, eyeZ.y, eyeZ.z);

            for(int i = 0; i < DIM; ++i) {
                if(M::IS::Intersects(ray, _bboxes[i])) {
                    _dragAxis = i;
                    _dragPlane = M::Plane::FromPointSpan(_cursorPos, M::Cross(eyeZ, Axis(i)), Axis(i));
                    printf("Cross: %f %f %f, Axis(i): %f %f %f\n",
                        M::Cross(eyeZ, Axis(i)).x,
                        M::Cross(eyeZ, Axis(i)).y,
                        M::Cross(eyeZ, Axis(i)).z,
                        Axis(i).x,
                        Axis(i).y,
                        Axis(i).z);
                    M::IS::Info inf;
                    bool is = M::IS::Intersects(ray, _dragPlane, &inf);
                    assert(is);
                    tmp = _cursorPos.vec[_dragAxis];
                    const std::vector<IGeometry*>& geomList = W::world.GetSelection()->GetList();
                    _oldPos.resize(geomList.size());
                    for(unsigned i = 0; i < _oldPos.size(); ++i) _oldPos[i] = ((const W::ENT_Geometry*)geomList[i])->Transform(M::Vector3::Zero);
                    _dragOrig = inf.where;
                    _dragging = true;
                }
            }
            if(_dragging) {
                printf("N = %f %f %f\n", _dragPlane.n.x, _dragPlane.n.y, _dragPlane.n.z);
                return true;
            } 
        } 
        if(!_dragging) {
            W::ENT_Geometry* geom;
            if(W::world.Trace(ray, &geom)) {
				if(MouseEvent::MODIFIER_SHIFT & event.mods) W::world.GetSelection()->Add(geom);
                else W::world.GetSelection()->Set(geom);
                return true;
            }
        }
        return false;
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

static inline M::Matrix4 SetZ(const M::Vector3& pos, float z) {
    M::Matrix4 m = M::Mat4::Identity();
    m.m22 = z / pos.z;
    return m;
}

bool Translate::OnMouseMove(const MouseEvent& event) {
    R::Color arrowHeadColors[] = {
        R::Color::Red,
        R::Color::Green,
        R::Color::Blue
    };
    if(!_dragging) {
		M::Ray ray = W::world.PickingRay(event.coords);
        for(int i = 0; i < DIM; ++i) {
            /* TODO: color arrow heads
            if(M::IS::Intersects(ray, _bboxes[i])) {
                leda::nb::set_color(_geom_arrowHeads[i]->GetRatPolyMesh(), R::BlendAddRGB(arrowHeadColors[i], R::Color::White));
            } else leda::nb::set_color(_geom_arrowHeads[i]->GetRatPolyMesh(), arrowHeadColors[i]);
            _geom_arrowHeads[i]->Update();
            */
        }
    }

    if(_dragging) {
        M::Ray ray = W::world.PickingRay(event.coords);
        M::IS::Info inf;
        bool is = M::IS::Intersects(ray, _dragPlane, &inf);
        assert(is);
        M::Vector3 p = inf.where;
        _cursorPos.vec[_dragAxis] = tmp + (p - _dragOrig).vec[_dragAxis];
        printf("orig = %f %f %f, p = %f %f %f\n",
            _dragOrig.x, _dragOrig.y, _dragOrig.z,
            p.x, p.y, p.z);
        printf("COORD = %f\n", _cursorPos.vec[_dragAxis]);
        SetPosition(_cursorPos);

        const std::vector<IGeometry*>& geomList = W::world.GetSelection()->GetList();
        for(unsigned i = 0; i < geomList.size(); ++i) {
            IGeometry* geom = geomList[i];
            M::Vector3 pos = _oldPos[i];
            pos.vec[_dragAxis] = _oldPos[i].vec[_dragAxis] + (p - _dragOrig).vec[_dragAxis];
            geom->SetPosition(pos.x, pos.y, pos.z);
        }

        AlignWithCamera();
        return true;
    }
    return false;
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