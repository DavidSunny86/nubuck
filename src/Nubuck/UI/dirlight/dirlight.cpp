#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>

#include <renderer\renderer.h>
#include <renderer\mesh\sphere\sphere.h>
#include <renderer\mesh\meshmgr.h>
#include <UI\renderstats\renderstats.h>
#include <UI\colorbutton\colorbutton.h>
#include <UI\dirlight\dirlight.h>
#include <world\world.h>

namespace UI {

// ================================================================================
// DirLight
// ================================================================================

void DirLight::SetupLights(R::RenderList& renderList) {
    renderList.dirLights[0].direction       = _lightDir;
    renderList.dirLights[0].diffuseColor    = _lightCol;

    renderList.dirLights[1].diffuseColor    = R::Color::Black;
    renderList.dirLights[2].diffuseColor    = R::Color::Black;
}

void DirLight::initializeGL() {
    // create sphere mesh
    R::Sphere sphere(5, true);
    _sphereMesh = R::meshMgr.Create(sphere.GetDesc());
    _sphereTFMesh = R::meshMgr.Create(_sphereMesh);

    R::meshMgr.GetMesh(_sphereTFMesh).SetTransform(M::Mat4::Identity());
}

void DirLight::resizeGL(int width, int height) {
    _camera.SetScreenSize(width, height);
    R::theRenderer.Resize(width, height);
}

void DirLight::paintGL() {
    R::RenderList renderList;

    renderList.Clear();
    SetupLights(renderList);
    renderList.worldMat = M::Mat4::Translate(0.0f, 0.0f, -4.0f);
    renderList.projWeight = 0.0f;

    R::Material mat = R::Material::White;
    mat.SetUniformBinding("patternColor", R::Color(0.0f, 0.0f, 0.0f, 0.0f));
    mat.SetUniformBinding("patternTex", NULL);

    R::MeshJob rjob;
    rjob.fx         = "LitDirectionalTwosided";
    rjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
    rjob.material   = mat;
    rjob.primType   = 0;
    rjob.tfmesh     = _sphereTFMesh;
    renderList.meshJobs.push_back(rjob);

    R::theRenderer.Resize(width(), height());
    R::theRenderer.BeginFrame();
    R::theRenderer.Render(renderList);
    R::theRenderer.EndFrame();

    std::string stats = R::theRenderer.GetFrameStats();
    UI::RenderStats::Instance()->Update(_renderContext, stats);
}

void DirLight::mousePressEvent(QMouseEvent* event) {
    if(Qt::LeftButton == event->button()) {
        _camera.StartDragging(event->x(), event->y());
        _isDragging = true;
    }
}

void DirLight::mouseReleaseEvent(QMouseEvent* event) {
    if(Qt::LeftButton == event->button()) {
        _camera.StopDragging();
        _isDragging = false;
    }
}

void DirLight::mouseMoveEvent(QMouseEvent* event) {
    _camera.Drag(event->x(), event->y());
    if(_isDragging) {
        const M::Matrix3 rot = M::RotationOf(_camera.GetWorldToEyeMatrix());
        SetDirection(M::Normalize(M::Transform(rot, M::Vector3(0.0f, 0.0f, -1.0f))));
        update();
    }
}

void DirLight::OnColorChanged(float r, float g, float b) {
    _lightCol = R::Color(r, g, b);
    update();
}

DirLight::DirLight()
    : _camera(100, 100)
    , _isDragging(false)
    , _lightDir(M::Vector3(0.0f, 0.0f, -1.0f))
    , _lightCol(R::Color(0.5f, 0.5f, 0.5f))
{
    _camera.ResetRotation();

    setMinimumSize(100, 100);
}

void DirLight::SetDirection(const M::Vector3& dir) {
    _lightDir = dir;
    emit SigDirectionChanged(_lightDir.x, _lightDir.y, _lightDir.z);
}

void DirLight::SetRenderContext(const std::string& context) {
    _renderContext = context;
}

QSize DirLight::sizeHint() const {
    return QSize(100, 100);
}

// ================================================================================
// DirLightControls
// ================================================================================

void DirLightControls::OnLightChanged() {
    R::DirectionalLight dirLight;
    dirLight.direction = _dirLight->GetDirection();
    dirLight.diffuseColor = _dirLight->GetColor();
    W::world.SetDirectionalLight(_lightIdx, dirLight);
}

DirLightControls::DirLightControls(const int lightIdx) : _lightIdx(lightIdx) {
    COM_assert(0 <= lightIdx && lightIdx < 3);

    const R::DirectionalLight& params = W::world.GetDirectionalLight(lightIdx);

    QVBoxLayout* vbox = new QVBoxLayout;

    std::string renderContexts[] = {
        "DirLightControl 0",
        "DirLightControl 1",
        "DirLightControl 2"
    };

    _dirLight = new DirLight;
    _dirLight->SetDirection(params.direction);
    _dirLight->SetRenderContext(renderContexts[_lightIdx]);

    connect(_dirLight, SIGNAL(SigDirectionChanged(float, float, float)),
        this, SLOT(OnLightChanged()));

    _colorButton = new ColorButton;
    _colorButton->SetColor(params.diffuseColor.r, params.diffuseColor.g, params.diffuseColor.b);

    connect(_colorButton, SIGNAL(SigColorChanged(float, float, float)),
        _dirLight, SLOT(OnColorChanged(float, float, float)));
    connect(_colorButton, SIGNAL(SigColorChanged(float, float, float)),
        this, SLOT(OnLightChanged()));

    QFrame* frame = new QFrame;
    frame->setObjectName("glframe");
    frame->setLayout(new QVBoxLayout);
    frame->layout()->addWidget(_dirLight);

    vbox->addWidget(frame);

    QFormLayout* form = new QFormLayout;

    QLabel* lblColor = new QLabel("color: ");
    lblColor->setStyleSheet("background-color: rgba(0, 0, 0, 0)");

    form->addRow(lblColor, _colorButton);

    vbox->addLayout(form);
    vbox->addSpacing(10);

    QString titles[] = { "Light 0", "Light 1", "Light 2" };

    QGroupBox* group = new QGroupBox(titles[lightIdx]);
    group->setObjectName("vectorGroup");
    group->setLayout(vbox);

    setLayout(new QVBoxLayout);
    layout()->addWidget(group);
}

} // namespace UI