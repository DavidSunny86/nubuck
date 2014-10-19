#include <QMouseEvent>
#include <QKeyEvent>

#include <Nubuck\common\common.h>
#include <system\opengl\opengl.h>
#include <UI\window_events.h>
#include <UI\outliner\outliner.h>
#include <world\world.h>
#include <operators\operators.h>
#include <renderer\metrics\metrics.h>
#include <renderer\effects\effectmgr.h>
#include <renderer\mesh\quad\quad.h>
#include <renderer\mesh\meshmgr.h>
#include "renderview.h"

#include <operators\operators.h> // !!!

namespace UI {

    // top-down gradient
    void RenderView::BuildBackgroundGradient() {
        if(_bgGradient.mesh) {
            R::meshMgr.Destroy(_bgGradient.mesh);
            _bgGradient.mesh = NULL;
        }
        if(_bgGradient.tfmesh) {
            R::meshMgr.Destroy(_bgGradient.tfmesh);
            _bgGradient.tfmesh = NULL;
        }

        R::Mesh::Desc meshDesc = R::CreateQuadDesc(2.0f);
        R::Color c0 = _bgColor;
        R::Color c1 = R::Lerp(c0, R::Color::Black, 0.6f);
        meshDesc.vertices[0].color = c1;
        meshDesc.vertices[1].color = c1;
        meshDesc.vertices[2].color = c0;
        meshDesc.vertices[3].color = c0;

        _bgGradient.mesh = R::meshMgr.Create(meshDesc);
        _bgGradient.tfmesh = R::meshMgr.Create(_bgGradient.mesh);

        R::meshMgr.GetMesh(_bgGradient.tfmesh).SetTransform(M::Mat4::Identity());
    }

    void RenderView::RenderBackgroundGradient(R::RenderList& renderList) {
        R::MeshJob mjob;
        mjob.fx         = "UnlitTP";
        mjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        mjob.tfmesh     = _bgGradient.tfmesh;
        mjob.material   = R::Material::White;
        mjob.primType   = 0;

        renderList.meshJobs.push_back(mjob);
    }

    void RenderView::initializeGL(void) {
        _renderer.Init();
        _debugText.Init(GetRenderingContext().GetDeviceContext());

        BuildBackgroundGradient();
    }

    void RenderView::resizeGL(int width, int height) {
        EV::Params_Resize args;
        args.width = width;
        args.height = height;
        W::world.Send(EV::def_Resize.Create(args));

        _renderer.Resize(width, height);
        _debugText.Resize(width, height);
        Render();
    }

    void RenderView::finishResizeGL() {
        _renderer.FinishResize();
    }

    bool RenderView::focusNextPrevChild(bool) {
        // do not switch to next widget in tab order.
        // generate KeyEvent with key = Key_Tab instead.
        return false;
    }

    void RenderView::enterEvent(QEvent* event) {
        setFocus(Qt::OtherFocusReason);
    }

    void RenderView::mousePressEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_DOWN;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        if(EV::Params_Mouse::BUTTON_RIGHT == args.button) {
            EV::Event event = EV::def_Mouse.Create(args);
            OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
        } else W::world.HandleMouseEvent(args);
    }

    void RenderView::mouseReleaseEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_UP;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        if(EV::Params_Mouse::BUTTON_RIGHT == args.button) {
            EV::Event event = EV::def_Mouse.Create(args);
            OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::ALWAYS);
        } else W::world.HandleMouseEvent(args);
    }

    void RenderView::mouseMoveEvent(QMouseEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_MOVE;
        args.button = qevent->button();
        args.mods = qevent->modifiers();
        args.x = qevent->x();
        args.y = qevent->y();
		args.ret = NULL;

        EV::Event event = EV::def_Mouse.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::DROP_WHEN_BUSY);
        W::world.HandleMouseEvent(args);
    }

    void RenderView::wheelEvent(QWheelEvent* qevent) {
        EV::Params_Mouse args;
        args.type = EV::Params_Mouse::MOUSE_WHEEL;
        args.mods = qevent->modifiers();
        args.delta = qevent->delta();
        args.x = qevent->x();
        args.y = qevent->y();

        W::world.HandleMouseEvent(args);
    }

    void RenderView::keyPressEvent(QKeyEvent* qevent) {
        // screenshot
        if(Qt::Key_F12 == qevent->key() && !qevent->isAutoRepeat()) {
            _renderer.Screenshot();
            return;
        }

        // large screenshot
        if(Qt::Key_F11 == qevent->key() && !qevent->isAutoRepeat()) {
            _largeScreenshotRequested = true;
            return;
        }

        if(Qt::Key_Tab == qevent->key() && !qevent->isAutoRepeat()) {
            W::world.GetEditMode().CycleModes();
            return;
        }

        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_DOWN;
        args.keyCode = qevent->key();
        args.nativeScanCode = qevent->nativeScanCode();
        args.autoRepeat = qevent->isAutoRepeat();
        args.mods = qevent->modifiers();


        EV::Event event = EV::def_Key.Create(args);
        OP::g_operators.InvokeAction(event, OP::Operators::InvokationMode::DROP_WHEN_BUSY);
    }

    void RenderView::keyReleaseEvent(QKeyEvent* qevent) {
        EV::Params_Key args;
        args.type = EV::Params_Key::KEY_UP;
        args.keyCode = qevent->key();
        if(!qevent->isAutoRepeat()) W::world.Send(EV::def_Key.Create(args));
    }

    void RenderView::OnSetBackgroundColor(float r, float g, float b) {
        _bgColor = R::Color(r, g, b);
        _renderer.SetClearColor(_bgColor);
        BuildBackgroundGradient();
    }

    void RenderView::OnSetBackgroundColor(const R::Color& color) {
        OnSetBackgroundColor(color.r, color.g, color.b);
    }

    void RenderView::OnSetBackgroundColor(const QColor& color) {
        OnSetBackgroundColor(color.redF(), color.greenF(), color.blueF());
    }

    void RenderView::OnShowBackgroundGradient(bool show) {
        _bgGradient.show = show;
    }

    void RenderView::OnShowBackgroundGradient(int show) {
        OnShowBackgroundGradient(0 != show);
    }

    R::Color RenderView::defaultBackgroundColor = R::Color::FromBytes(154, 206, 235); // cornflower blue (crayola)

    RenderView::RenderView(QWidget* parent)
        : glWidget_t(parent)
        , _fpsLabel(NULL)
        , _time(0.0f)
        , _bgColor(defaultBackgroundColor)
    {
        setFocusPolicy(Qt::StrongFocus);
        setMouseTracking(true);

        _largeScreenshotRequested = false;
    }

    RenderView::~RenderView(void) {
		R::effectMgr.FreeResources();
    }

    const R::Renderer& RenderView::GetRenderer() const {
        return _renderer;
    }

    const R::Color& RenderView::GetBackgroundColor() const {
        return _bgColor;
    }

    bool RenderView::IsBackgroundGradient() const {
        return _bgGradient.show;
    }

    void RenderView::Render() {
        _time += M::Max(0.0f, _rtimer.Stop());
        if(1.0f <= _time) {
            float fps = _numFrames / _time;
            _numFrames = 0;
            _time = 0.0f;

            if(_fpsLabel) _fpsLabel->setText(QString("fps = %1").arg(fps));
        }
        _numFrames++;
        _rtimer.Start();

        _renderList.Clear();
        if(_bgGradient.show) RenderBackgroundGradient(_renderList);
        W::world.Render(_renderList);
        OP::g_operators.GetMeshJobs(_renderList.meshJobs);

        if(_largeScreenshotRequested) {
            _renderer.LargeScreenshot(10000, 10000, _renderList);
            _largeScreenshotRequested = false;
        }

        _renderer.BeginFrame();
        _renderer.Render(_renderList);
        _renderer.EndFrame();

        _debugText.BeginFrame();
        _debugText.Printf("frame time: %f\n", R::metrics.frame.time);
        _debugText.Printf("number of draw calls: %d\n", R::metrics.frame.numDrawCalls);
        _debugText.Printf("operator driver queue size: %d\n", OP::g_operators.GetDriverQueueSize());
        _debugText.EndFrame();

        updateGL();
    }

    QLabel* RenderView::FpsLabel(void) {
        if(!_fpsLabel) _fpsLabel = new QLabel("fps = ?");
        return _fpsLabel;
    }

} // namespace UI
