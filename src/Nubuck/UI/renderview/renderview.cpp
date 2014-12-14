#include <QMouseEvent>
#include <QKeyEvent>

#include <Nubuck\common\common.h>
#include <Nubuck\events\core_events.h>
#include <system\opengl\opengl.h>
#include <UI\window_events.h>
#include <UI\outliner\outliner.h>
#include <UI\penoptions\pen_options.h>
#include <UI\userinterface.h>
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

    // p in widget relative coordinates, as returned by QMouseEvent::posF()
    void RenderView::EmitPenVertex(const QPointF& p) {
        R::PenVertex pv;
        pv.pos = M::Vector2(p.x(), height() - p.y());
        pv.col = g_ui.GetPenOptions().GetColor();
        pv.size = g_ui.GetPenOptions().GetSize();
        _pen.push_back(pv);
    }

    void RenderView::initializeGL(void) {
        _renderer.Init();
        _debugText.Init(GetRenderingContext().GetDeviceContext());

        BuildBackgroundGradient();
    }

    void RenderView::resizeGL(int width, int height) {
        EV::ResizeEvent event;
        event.width = width;
        event.height = height;
        W::world.Send(ev_resize, event);

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
        EV::MouseEvent event;
        event.type = EV::MouseEvent::MOUSE_DOWN;
        event.button = qevent->button();
        event.mods = qevent->modifiers();
        event.x = qevent->x();
        event.y = qevent->y();
		event.ret = NULL;

        if(EV::MouseEvent::BUTTON_RIGHT == event.button) {
            OP::g_operators.InvokeAction(ev_mouse, event, OP::Operators::InvokationMode::ALWAYS);
        } else {
            if(CAMERA == _inputMode) W::world.HandleMouseEvent(event);
            else {
                assert(PEN == _inputMode);
                _isPenDown = true;
                EmitPenVertex(qevent->posF());
            }
        }
    }

    void RenderView::mouseReleaseEvent(QMouseEvent* qevent) {
        EV::MouseEvent event;
        event.type = EV::MouseEvent::MOUSE_UP;
        event.button = qevent->button();
        event.mods = qevent->modifiers();
        event.x = qevent->x();
        event.y = qevent->y();
		event.ret = NULL;

        if(EV::MouseEvent::BUTTON_RIGHT == event.button) {
            OP::g_operators.InvokeAction(ev_mouse, event, OP::Operators::InvokationMode::ALWAYS);
        } else {
            if(CAMERA == _inputMode) W::world.HandleMouseEvent(event);
            else {
                assert(PEN == _inputMode);
                _isPenDown = false;
                _pen.push_back(R::Pen_RestartVertex());
            }
        }
    }

    void RenderView::mouseMoveEvent(QMouseEvent* qevent) {
        EV::MouseEvent event;
        event.type = EV::MouseEvent::MOUSE_MOVE;
        event.button = qevent->button();
        event.mods = qevent->modifiers();
        event.x = qevent->x();
        event.y = qevent->y();
		event.ret = NULL;

        OP::g_operators.InvokeAction(ev_mouse, event, OP::Operators::InvokationMode::DROP_WHEN_BUSY);
        if(CAMERA == _inputMode) W::world.HandleMouseEvent(event);
        else if(_isPenDown) {
            assert(PEN == _inputMode);
            EmitPenVertex(qevent->posF());
        }
    }

    void RenderView::wheelEvent(QWheelEvent* qevent) {
        EV::MouseEvent event;
        event.type = EV::MouseEvent::MOUSE_WHEEL;
        event.mods = qevent->modifiers();
        event.delta = qevent->delta();
        event.x = qevent->x();
        event.y = qevent->y();

        W::world.HandleMouseEvent(event);
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

        // toggle pen
        if(Qt::Key_P == qevent->key() && !qevent->isAutoRepeat()) {
            _inputMode = InputMode(1 - _inputMode);
            if(_isPenDown) {
                _pen.push_back(R::Pen_RestartVertex());
                _isPenDown = false;
            }
            return;
        }

        // clear pen (pen mode only)
        // TODO: Key_C conflicts with shortcut for convex hull
        if(PEN == _inputMode && Qt::Key_X == qevent->key() && !qevent->isAutoRepeat()) {
            _pen.clear();
            return;
        }

        EV::KeyEvent event;
        event.type = EV::KeyEvent::KEY_DOWN;
        event.keyCode = qevent->key();
        event.nativeScanCode = qevent->nativeScanCode();
        event.autoRepeat = qevent->isAutoRepeat();
        event.mods = qevent->modifiers();


        OP::g_operators.InvokeAction(ev_key, event, OP::Operators::InvokationMode::DROP_WHEN_BUSY);
    }

    void RenderView::keyReleaseEvent(QKeyEvent* qevent) {
        EV::KeyEvent event;
        event.type = EV::KeyEvent::KEY_UP;
        event.keyCode = qevent->key();
        if(!qevent->isAutoRepeat()) W::world.Send(ev_key, event);
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
        , _inputMode(CAMERA)
        , _isPenDown(false)
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
        _renderer.RenderPen(_pen);
        _renderer.EndFrame();

        _debugText.BeginFrame();
        _debugText.Printf("frame time: %f\n", R::metrics.frame.time);
        _debugText.Printf("number of draw calls: %d\n", R::metrics.frame.numDrawCalls);
        _debugText.Printf("operator driver queue size: %d\n", OP::g_operators.GetDriverQueueSize());
        _debugText.Printf(" \n");
        if(PEN == _inputMode) _debugText.Printf("--- USING PEN");
        _debugText.EndFrame();

        updateGL();
    }

    QLabel* RenderView::FpsLabel(void) {
        if(!_fpsLabel) _fpsLabel = new QLabel("fps = ?");
        return _fpsLabel;
    }

} // namespace UI
