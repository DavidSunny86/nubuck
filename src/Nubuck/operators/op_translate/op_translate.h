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
    TranslatePanel() { }
};

class Translate : public Operator {
private:
    typedef leda::rational      scalar_t;
    typedef leda::d3_rat_point  point3_t;

    nb::transform_gizmo _gizmo;

    void UpdateCursor();

    std::vector<M::Vector3> 		_oldEntityPos;
    leda::node_array<M::Vector3>    _oldVertPos;
    leda::node_array<R::Color>      _oldVertCol;
    M::Vector3                      _center;
    W::editMode_t::Enum             _editMode;

    bool DoPicking(const MouseEvent& event);
    void OnBeginDragging();
    void OnDragging(const Nubuck::transform_gizmo_mouse_info& info);

    void Event_SelectionChanged(const EV::Event& event) {
        OnGeometrySelected();
	}
public:
    Translate();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }

    void OnGeometrySelected() override;
    void OnEditModeChanged(const W::editMode_t::Enum mode) override;
    bool OnMouse(const MouseEvent& event) override;
    bool OnKey(const KeyEvent& event) override;
};

} // namespace OP