#pragma once

#include <string>
#include <vector>

#include <Windows.h> // defines MAX_INT
#include <LEDA/geo/d3_rat_point.h>
#include <LEDA/graph/graph.h>

#include <Nubuck\nubuck_api.h>
#include <Nubuck\polymesh_fwd.h>
#include <Nubuck\math\vector3.h>

class QMenu;
class QWidget;

namespace M {

struct Vector2;

} // namespace M

namespace R {

struct Color;

} // namespace R

namespace W {

class Entity;
class ENT_Geometry;
class ENT_Text;
class ENT_TransformGizmo;

} // namespace W

namespace OP {

struct MouseEvent;

} // namespace OP

namespace nb {

typedef W::Entity*              entity;
typedef W::ENT_Geometry*        geometry;
typedef W::ENT_Text*            text;
typedef W::ENT_TransformGizmo*  transform_gizmo;

struct EntityType {
    enum Enum {
        GEOMETRY = 0,
        TEXT
    };
};

} // namespace nb

struct Nubuck {
    // logfile
    virtual void log_printf(const char* format, ...) = 0;

    // user interface
    virtual QMenu*  scene_menu() = 0;
    virtual QMenu*  object_menu() = 0;
    virtual QMenu*  algorithm_menu() = 0;
    virtual QMenu*  vertex_menu() = 0;
    virtual void    set_operator_name(const char* name) = 0;
    virtual void    set_operator_panel(QWidget* panel) = 0;

    // world
    virtual void                destroy(const nb::entity obj) = 0;
    virtual nb::geometry        create_geometry() = 0;
    virtual void                destroy_geometry(const nb::geometry obj) = 0;
    virtual nb::text            create_text() = 0;
    virtual nb::transform_gizmo create_transform_gizmo() = 0;

    // selection
    enum SelectMode {
        SELECT_MODE_NEW,
        SELECT_MODE_ADD
    };

    virtual void                    clear_selection() = 0;
    virtual void                	select_geometry(SelectMode mode, const nb::geometry obj) = 0;
    virtual void                    select(SelectMode mode, const nb::entity obj) = 0;
    virtual void                	select_vertex(SelectMode, const nb::geometry obj, const leda::node vert) = 0;

    virtual std::vector<nb::geometry>   selected_geometry() = 0;
    virtual M::Vector3                  global_center_of_selection() = 0;

    // entities
    virtual M::Vector3              position(nb::entity obj) = 0;

    virtual nb::EntityType::Enum    type_of(nb::entity obj) = 0;
    virtual nb::geometry            to_geometry(nb::entity obj) = 0;
    virtual nb::text                to_text(nb::entity obj) = 0;

    virtual nb::entity              first_selected_entity() = 0;
    virtual nb::entity              next_selected_entity(nb::entity obj) = 0;

    virtual void                    set_position(nb::entity obj, const M::Vector3& pos) = 0;

    // geometry
    struct RenderMode {
        enum Flags {
            FACES    = (1 << 0),
            NODES    = (1 << 1),
            EDGES    = (1 << 2)
        };
    };

    struct ShadingMode {
        enum Enum {
            NICE = 0,
            FAST,
            LINES,
            NICE_BILLBOARDS,

            NUM_MODES
        };
    };

    struct Pattern {
        enum Enum {
            NONE,
            CHECKER,
            DOTS,
            LINES
        };
    };

    virtual const std::string&  geometry_name(const nb::geometry obj) = 0;
    virtual M::Vector3          geometry_position(const nb::geometry obj) = 0;

    virtual nb::geometry        first_selected_geometry() = 0;
    virtual nb::geometry        next_selected_geometry(nb::geometry obj) = 0;

    virtual leda::nb::RatPolyMesh& poly_mesh(const nb::geometry obj) = 0;

    virtual void                set_geometry_name(const nb::geometry obj, const std::string& name) = 0;

    virtual void                apply_geometry_transformation(const nb::geometry obj) = 0;

    virtual void                set_geometry_position(const nb::geometry obj, const M::Vector3& position) = 0;
    virtual void                set_geometry_scale(const nb::geometry obj, const M::Vector3& scale) = 0;

    virtual void                hide_geometry_outline(const nb::geometry obj) = 0;

    virtual void                hide_geometry(const nb::geometry obj) = 0;
    virtual void                show_geometry(const nb::geometry obj) = 0;

    virtual void                set_geometry_solid(const nb::geometry obj, bool solid) = 0;
    virtual void                set_geometry_render_mode(const nb::geometry obj, int flags) = 0;
    virtual void                set_geometry_render_layer(const nb::geometry obj, unsigned layer) = 0;
    virtual void                set_geometry_shading_mode(const nb::geometry obj, ShadingMode::Enum mode) = 0;
    virtual void                set_geometry_pattern(const nb::geometry obj, const Pattern::Enum pattern) = 0;
    virtual void                set_geometry_pattern_color(const nb::geometry obj, const R::Color& color) = 0;

    // text
    virtual const M::Vector2&   text_content_size(const nb::text obj) const = 0;

    virtual void                set_text_position(const nb::text obj, const M::Vector3& pos) = 0;
    virtual void                set_text_content(const nb::text obj, const std::string& content) = 0;
    virtual void                set_text_content_scale(const nb::text obj, const char refChar, const float refCharSize) = 0;

    // transform gizmo
    struct TransformGizmoMode {
        enum Enum {
            TRANSLATE = 0,
            SCALE,
            ROTATE,
            NUM_MODES
        };
    };

    struct transform_gizmo_action {
        enum Enum {
            NO_ACTION,
            BEGIN_DRAGGING,
            END_DRAGGING,
            DRAGGING
        };
    };

    struct transform_gizmo_axis {
        enum Enum {
            X, Y, Z, INVALID_AXIS
        };
    };

    struct transform_gizmo_mouse_info {
        transform_gizmo_action::Enum    action;
        transform_gizmo_axis::Enum      axis;
        float                           value; // either translation or scale
    };

    virtual TransformGizmoMode::Enum transform_gizmo_mode(const nb::transform_gizmo obj) = 0;

    virtual void                set_transform_gizmo_mode(const nb::transform_gizmo obj, TransformGizmoMode::Enum mode) = 0;

    virtual void                set_transform_gizmo_position(const nb::transform_gizmo obj, const M::Vector3& pos) = 0;

    virtual void                hide_transform_gizmo(const nb::transform_gizmo obj) = 0;
    virtual void                show_transform_gizmo(const nb::transform_gizmo obj) = 0;

    virtual nb::transform_gizmo global_transform_gizmo() = 0;
    virtual bool                transform_gizmo_handle_mouse_event(
        const nb::transform_gizmo obj,
        const OP::MouseEvent& event,
        transform_gizmo_mouse_info& info) = 0;

    // animation
    virtual void wait_for_animations() = 0;
};

NUBUCK_API Nubuck& nubuck();

struct IPhase {
    enum StepRet {
        DONE = 0,
        CONTINUE
    };

    virtual ~IPhase(void) { }

    virtual void Enter(void) = 0;
    virtual void Leave(void) = 0;

    virtual bool IsWall(void) const = 0;
    virtual bool IsDone(void) const = 0;

    virtual StepRet     Step(void) = 0;
    virtual IPhase*     NextPhase(void) = 0;

    virtual void        OnNodesMoved(void) = 0;
    virtual void        OnKeyPressed(char c) = 0;
};

struct IAlgorithm {
    virtual ~IAlgorithm(void) { }

    virtual IPhase* Init(const Nubuck& nubuck, const leda::GRAPH<leda::d3_rat_point, int>& G) = 0;

    // return false if the algorithm implements no special Run method
    virtual bool Run(void) = 0;
};

typedef IAlgorithm* (*algAlloc_t)(void);

template<typename ALGORITHM>
IAlgorithm* CreateAlgorithm(void) {
    return new ALGORITHM();
}

// main entry point, returns exit code
NUBUCK_API int RunNubuck(int argc, char* argv[], algAlloc_t algAlloc);
