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
    nb::text        create_text();

    void                    clear_selection();
    void                	select_geometry(SelectMode mode, const nb::geometry obj);
    void                    select(SelectMode mode, const nb::entity obj);
    void                	select_vertex(SelectMode, const nb::geometry obj, const leda::node vert);

    std::vector<nb::geometry>   selected_geometry();
    M::Vector3                  global_center_of_selection();

    // entities
    M::Vector3              position(nb::entity obj);

    nb::EntityType::Enum    type_of(nb::entity obj);
    nb::geometry            to_geometry(nb::entity obj);
    nb::text                to_text(nb::entity obj);

    nb::entity              first_selected_entity();
    nb::entity              next_selected_entity(nb::entity obj);

    void                    set_position(nb::entity obj, const M::Vector3& pos);

    // geometry
    const std::string&  geometry_name(const nb::geometry obj);
    M::Vector3          geometry_position(const nb::geometry obj);

    nb::geometry        first_selected_geometry();
    nb::geometry        next_selected_geometry(nb::geometry obj);

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

    // text
    const M::Vector2&   text_content_size(const nb::text obj) const;

    void                set_text_position(const nb::text obj, const M::Vector3& position);
    void                set_text_content(const nb::text obj, const std::string& content);
    void                set_text_content_scale(const nb::text obj, const char refChar, const float refCharSize);
};

extern NubuckImpl g_nubuck;