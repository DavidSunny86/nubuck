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
    void    add_menu_item(QMenu* menu, const char* name, OP::Invoker& invoker);
    void    set_operator_name(const char* name);
    void    set_operator_panel(QWidget* panel);

    // world
    void                destroy(const nb::entity obj);
#pragma region DEPRECATED
    nb::geometry        create_geometry();
    void                destroy_geometry(const nb::geometry obj);
#pragma endregion
    nb::mesh            create_mesh();
    void                destroy_mesh(const nb::mesh obj);
    nb::text            create_text();
    nb::transform_gizmo create_transform_gizmo();

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
    nb::mesh                to_mesh(nb::entity obj);
    nb::text                to_text(nb::entity obj);

    nb::entity              first_selected_entity();
    nb::entity              next_selected_entity(nb::entity obj);

    void                    set_position(nb::entity obj, const M::Vector3& pos);

#pragma region DEPRECATED
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
    void                set_geometry_pattern(const nb::geometry obj, const Pattern::Enum pattern);
    void                set_geometry_pattern_color(const nb::geometry obj, const R::Color& color);
#pragma endregion

    // mesh
    const std::string&  mesh_name(const nb::geometry obj);
    M::Vector3          mesh_position(const nb::geometry obj);

    nb::geometry        first_selected_mesh();
    nb::geometry        next_selected_mesh(nb::mesh obj);

    leda::NbGraph&      graph_of(const nb::mesh obj);

    void                set_mesh_name(const nb::mesh obj, const std::string& name);

    void                apply_mesh_transformation(const nb::mesh obj);

    void                set_mesh_position(const nb::mesh obj, const M::Vector3& position);
    void                set_mesh_scale(const nb::mesh obj, const M::Vector3& scale);

    void                hide_mesh_outline(const nb::mesh obj);

    void                hide_mesh(const nb::mesh obj);
    void                show_mesh(const nb::mesh obj);

    void                set_mesh_solid(const nb::mesh obj, bool solid);
    void                set_mesh_render_mode(const nb::mesh obj, int flags);
    void                set_mesh_render_layer(const nb::mesh obj, unsigned layer);
    void                set_mesh_shading_mode(const nb::mesh obj, ShadingMode::Enum mode);
    void                set_mesh_pattern(const nb::mesh obj, const Pattern::Enum pattern);
    void                set_mesh_pattern_color(const nb::mesh obj, const R::Color& color);

    // text
    const M::Vector2&   text_content_size(const nb::text obj) const;

    void                set_text_position(const nb::text obj, const M::Vector3& position);
    void                set_text_content(const nb::text obj, const std::string& content);
    void                set_text_content_scale(const nb::text obj, const char refChar, const float refCharSize);

    // transform gizmo
    TransformGizmoMode::Enum transform_gizmo_mode(const nb::transform_gizmo obj);

    void                set_transform_gizmo_mode(const nb::transform_gizmo obj, TransformGizmoMode::Enum mode);

    void                set_transform_gizmo_position(const nb::transform_gizmo obj, const M::Vector3& pos);

    void                hide_transform_gizmo(const nb::transform_gizmo obj);
    void                show_transform_gizmo(const nb::transform_gizmo obj);

    nb::transform_gizmo global_transform_gizmo();
    bool                transform_gizmo_handle_mouse_event(
        const nb::transform_gizmo obj,
        const OP::MouseEvent& event,
        transform_gizmo_mouse_info& info);

    // animation
    void wait_for_animations();
};

extern NubuckImpl g_nubuck;