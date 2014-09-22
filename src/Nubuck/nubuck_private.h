#pragma once

#include <Nubuck\nubuck.h>

extern bool g_showRenderViewControls; // wireframe, modify gizmo, etc.

void ToggleRenderViewControls();

struct NubuckImpl : Nubuck {
    // logfile
    void log_printf(const char* format, ...);

    // user interface
    QMenu*  scene_menu();
    QMenu*  object_menu();
    QMenu*  algorithm_menu();
    QMenu*  vertex_menu();
    void    set_operator_name(const char* name);
    void    set_operator_panel(QWidget* panel);

    // world
    nb::geometry    create_geometry();
    void            destroy_geometry(const nb::geometry obj);

    void                    clear_selection();
    void                	select_geometry(SelectMode mode, const nb::geometry obj);
    void                	select_vertex(SelectMode, const nb::geometry obj, const leda::node vert);

    std::vector<nb::geometry>   selected_geometry();
    M::Vector3                  global_center_of_selection();

    // geometry
    const std::string&  geometry_name(const nb::geometry obj);
    M::Vector3          geometry_position(const nb::geometry obj);

    leda::nb::RatPolyMesh& poly_mesh(const nb::geometry obj);

    void                set_geometry_name(const nb::geometry obj, const std::string& name);

    void                apply_geometry_transformation(const nb::geometry obj);

    void                set_geometry_position(const nb::geometry obj, const M::Vector3& position);
    void                set_geometry_scale(const nb::geometry obj, const M::Vector3& scale);

    void                hide_geometry_outline(const nb::geometry obj);

    void                hide_geometry(const nb::geometry obj);
    void                show_geometry(const nb::geometry obj);

    void                set_geometry_solid(const nb::geometry obj, bool solid);
    void                set_geometry_render_mode(const nb::geometry obj, int flags);
    void                set_geometry_render_layer(const nb::geometry obj, unsigned layer);
    void                set_geometry_shading_mode(const nb::geometry obj, ShadingMode::Enum mode);
};

extern NubuckImpl g_nubuck;