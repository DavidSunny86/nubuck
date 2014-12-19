#include <renderer\renderer.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\meshmgr.h>
#include <UI\dirlight\dirlight.h>

namespace UI {

void DirLight::initializeGL() {
    _renderer.Init();

    // create sphere mesh
    R::Sphere sphere(5, true);
    _sphereMesh = R::meshMgr.Create(sphere.GetDesc());
    _sphereTFMesh = R::meshMgr.Create(_sphereMesh);

    R::meshMgr.GetMesh(_sphereTFMesh).SetTransform(M::Mat4::Identity());
}

void DirLight::resizeGL(int width, int height) {
    _renderer.Resize(width, height);
}

static void SetupLights(R::RenderList& renderList) {
    const R::Color gray = R::Color(0.5f, 0.5f, 0.5f);

    renderList.dirLights[0].direction      = M::Vector3(-1.0f,  1.0f,  0.0f);
    renderList.dirLights[0].diffuseColor   = gray;

    renderList.dirLights[1].direction      = M::Vector3( 1.0f,  1.0f,  0.0f);
    renderList.dirLights[1].diffuseColor   = gray;

    renderList.dirLights[2].direction      = M::Vector3( 0.0f, -0.5f, -1.5f);
    renderList.dirLights[2].diffuseColor   = R::Color::White;
}

void DirLight::paintGL() {
    R::RenderList renderList;

    renderList.Clear();
    SetupLights(renderList);
    renderList.worldMat = M::Mat4::Identity();
    renderList.projWeight = 0.0f;

    R::MeshJob meshJob;
    meshJob.fx = "Unlit";
    meshJob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
    meshJob.material = R::Material::White;
    meshJob.primType = 0;
    meshJob.tfmesh = _sphereTFMesh;
    renderList.meshJobs.push_back(meshJob);

    /*
    _renderer.BeginFrame();
    _renderer.Render(renderList);
    _renderer.EndFrame();
    */
}

QSize DirLight::sizeHint() const {
    return QSize(100, 100);
}

} // namespace UI