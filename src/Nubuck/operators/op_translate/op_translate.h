#pragma once

#include <vector>

#include <QMenu>

#include <Nubuck\nubuck.h>
#include <Nubuck\math\box.h>
#include <Nubuck\math\plane.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>
#include <world\world_events.h>

#include <LEDA\geo\d3_hull.h>

// forward decls
namespace M { namespace IS { struct Info; } } 

namespace OP {

class TranslatePanel : public OperatorPanel {
public:
    TranslatePanel(QWidget* parent = NULL) : OperatorPanel(parent) { }
};

class Translate : public Operator {
private:
    typedef leda::rational      scalar_t;
    typedef leda::d3_rat_point  point3_t;

    struct TransformMode {
        enum Enum {
            TRANSLATE = 0,
            SCALE,
            ROTATE,
            NUM_MODES
        };
    };

    Nubuck _nb;

    TransformMode::Enum _mode;

    R::meshPtr_t    _axisMesh;
    R::tfmeshPtr_t  _axisTFMesh;

    enum { X = 0, Y, Z, DIM };
    R::meshPtr_t    _arrowHeadMeshes[3];
    R::tfmeshPtr_t  _arrowHeadTFMeshes[3];
    M::Matrix4      _arrowHeadTF[3];

    R::meshPtr_t    _boxHeadMeshes[3];
    R::tfmeshPtr_t  _boxHeadTFMeshes[3];
    M::Matrix4      _boxHeadTF[3];

    void BuildAxis();
    void BuildArrowHeads();
    void BuildBoxHeads();

    M::Box _bboxes[DIM]; // bounding boxes of cursor

    void BuildBBoxes();

    bool _hidden;

    void ShowCursor();
    void HideCursor();

    void UpdateCursor();

    void SetRenderPosition(const M::Vector3& pos);
    void SetCursorPosition(const M::Vector3& pos);

    M::Vector3                      _cursorPos; // use cursorPos as readonly and SetCursorPosition() for writes
    M::Vector3              		_oldCursorPos;
    std::vector<M::Vector3> 		_oldGeomPos;
    leda::node_array<M::Vector3>    _oldVertPos;
    M::Vector3                      _center;
    W::editMode_t::Enum             _editMode;

    bool        _dragging;
    int         _dragAxis;
    M::Vector3  _dragOrig;
    M::Plane    _dragPlane;

    bool TraceCursor(const M::Ray& ray, int& axis, M::IS::Info* inf = NULL);

    bool OnMouseDown(const MouseEvent& event);
    bool OnMouseUp(const MouseEvent& event);
    bool OnMouseMove(const MouseEvent& event);

    void Event_SelectionChanged(const EV::Event& event) {
        OnGeometrySelected();
	}
public:
    Translate();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) override;
    void OnGeometrySelected() override;
    void OnCameraChanged() override;
    void OnEditModeChanged(const W::editMode_t::Enum mode) override;
    bool OnMouse(const MouseEvent& event) override;
    bool OnKey(const KeyEvent& event) override;
};

} // namespace OP