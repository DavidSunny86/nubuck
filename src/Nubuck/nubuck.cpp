#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\operators\operator_invoker.h>

#include <UI\userinterface.h>
#include <UI\logwidget\logwidget.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\nb_widgets.h>

#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>

namespace NB {

NUBUCK_API void LogPrintf(const char* format, ...) {
    static char buffer[2048];

    memset(buffer, 0, sizeof(buffer));
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    UI::logWidget().printf(buffer);
}

NUBUCK_API QMenu* SceneMenu() {
    return g_ui.GetMainWindow().GetSceneMenu();
}

NUBUCK_API QMenu* ObjectMenu() {
    return g_ui.GetMainWindow().GetObjectMenu();
}

NUBUCK_API QMenu* AlgorithmMenu() {
    return g_ui.GetMainWindow().GetAlgorithmMenu();
}

NUBUCK_API QMenu* VertexMenu() {
    return g_ui.GetMainWindow().GetVertexMenu();
}

NUBUCK_API void AddMenuItem(QMenu* menu, const char* name, OP::Invoker& invoker) {
    QAction* action = menu->addAction(name);
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

NUBUCK_API void SetOperatorName(const char* name) {
    return g_ui.GetMainWindow().SetOperatorName(name);
}

NUBUCK_API void SetOperatorPanel(QWidget* panel) {
    return g_ui.GetMainWindow().SetOperatorPanel(panel);
}

NUBUCK_API Widget CastToWidget(Button button) {
    return button;
}

NUBUCK_API BoxLayout CreateHorizontalBoxLayout() {
    return new QHBoxLayout;
}

NUBUCK_API BoxLayout CreateVerticalBoxLayout() {
    return new QVBoxLayout;
}

NUBUCK_API Button CreateButton(unsigned id, const char* name) {
    return new NBW_Button(id, name);
}

NUBUCK_API void AddWidgetToBox(BoxLayout layout, Widget widget) {
    layout->addWidget(widget);
}

NUBUCK_API void DestroyEntity(Entity obj) {
    obj->Destroy();
}

NUBUCK_API Mesh CreateMesh() {
    return W::world.CreateGeometry();
}

NUBUCK_API void DestroyMesh(Mesh obj) {
    obj->Destroy();
}

NUBUCK_API Text CreateText() {
    return W::world.CreateText();
}

NUBUCK_API TransformGizmo CreateTransformGizmo() {
    return W::world.CreateTransformGizmo();
}

NUBUCK_API void ClearSelection() {
    W::world.ClearSelection();
}

NUBUCK_API void SelectMesh(SelectMode mode, Mesh obj) {
    if(SM_NEW == mode) W::world.Select_New(obj);
    else W::world.Select_Add(obj);
}

NUBUCK_API void SelectEntity(SelectMode mode, Entity obj) {
    if(SM_NEW == mode) W::world.Select_New(obj);
    else W::world.Select_Add(obj);
}

NUBUCK_API void SelectVertex(SelectMode mode, Mesh obj, const leda::node vert) {
    if(SM_NEW == mode) W::world.SelectVertex_New(obj, vert);
    else W::world.SelectVertex_Add(obj, vert);
}

NUBUCK_API M::Vector3 GlobalCenterOfSelection() {
    return W::world.GlobalCenterOfSelection();
}

NUBUCK_API M::Vector3 GetEntityPosition(Entity obj) {
    return obj->GetPosition();
}

NUBUCK_API EntityType GetType(Entity obj) {
    return EntityType(obj->GetType()); // types are compatible
}

NUBUCK_API Mesh CastToMesh(Entity obj) {
    assert(W::EntityType::ENT_GEOMETRY == obj->GetType());
    return static_cast<W::ENT_Geometry*>(obj); // ugly but safe downcast
}

NUBUCK_API Text CastToText(Entity obj) {
    assert(W::EntityType::ENT_TEXT == obj->GetType());
    return static_cast<W::ENT_Text*>(obj); // ugly but safe downcast
}

NUBUCK_API Entity FirstSelectedEntity() {
    return W::world.FirstSelectedEntity();
}

NUBUCK_API Entity NextSelectedEntity(Entity obj) {
    return W::world.NextSelectedEntity(obj);
}

NUBUCK_API void SetEntityPosition(Entity obj, const M::Vector3& pos) {
    obj->SetPosition(pos);
}

NUBUCK_API const std::string& GetMeshName(Mesh obj) {
    return obj->GetName();
}

NUBUCK_API M::Vector3 GetMeshPosition(Mesh obj) {
    return obj->GetPosition();
}

NUBUCK_API Mesh FirstSelectedMesh() {
    W::Entity* ent = W::world.FirstSelectedEntity();
    while(ent) {
        if(W::EntityType::ENT_GEOMETRY == ent->GetType()) {
            return static_cast<W::ENT_Geometry*>(ent); // ugly but safe downcast
        }
        ent = ent->selectionLink.next;
    }
    return NULL;
}

NUBUCK_API Mesh NextSelectedMesh(Mesh obj) {
    W::Entity* ent = obj->selectionLink.next;
    while(ent) {
        if(W::EntityType::ENT_GEOMETRY == ent->GetType()) {
            return static_cast<W::ENT_Geometry*>(ent); // ugly but safe downcast
        }
        ent = ent->selectionLink.next;
    }
    return NULL;
}

NUBUCK_API void SetGraph(Mesh obj, const leda::NbGraph& graph) {
    obj->GetRatPolyMesh() = graph;
}

NUBUCK_API leda::NbGraph& GetGraph(Mesh obj) {
    return obj->GetRatPolyMesh();
}

NUBUCK_API void SetMeshName(Mesh obj, const std::string& name) {
    obj->SetName(name);
}

NUBUCK_API void ApplyMeshTransformation(Mesh obj) {
    obj->ApplyTransformation();
}

NUBUCK_API void SetMeshPosition(Mesh obj, const M::Vector3& position) {
    obj->SetPosition(position);
}

NUBUCK_API void SetMeshScale(Mesh obj, const M::Vector3& scale) {
    obj->SetScale(scale);
}

NUBUCK_API void HideMeshOutline(Mesh obj) {
    obj->HideOutline();
}

NUBUCK_API void HideMesh(Mesh obj) {
    obj->Hide();
}

NUBUCK_API void ShowMesh(Mesh obj) {
    obj->Show();
}

NUBUCK_API void SetMeshSolid(Mesh obj, bool solid) {
    obj->SetSolid(solid);
}

NUBUCK_API void SetMeshRenderMode(Mesh obj, int flags) {
    obj->SetRenderMode(flags);
}

NUBUCK_API void SetMeshRenderLayer(Mesh obj, unsigned layer) {
    obj->SetRenderLayer(layer);
}

NUBUCK_API void SetMeshShadingMode(Mesh obj, ShadingMode mode) {
    obj->SetShadingMode(mode);
}

NUBUCK_API void SetMeshPattern(Mesh obj, const Pattern pattern) {
    obj->SetPattern(pattern);
}

NUBUCK_API void SetMeshPatternColor(Mesh obj, const R::Color& color) {
    obj->SetPatternColor(color);
}

NUBUCK_API void ShowMeshVertexLabels(Mesh mesh, bool show) {
    mesh->Send(ev_geom_showVertexLabels.Tag(show));
}

NUBUCK_API const M::Vector2& GetTextContentSize(Text obj) {
    return obj->GetContentSize();
}

NUBUCK_API void SetTextPosition(Text obj, const M::Vector3& position) {
    obj->SetPosition(position);
}

NUBUCK_API void SetTextContent(Text obj, const std::string& content) {
    obj->SetContent(content);
}

NUBUCK_API void SetTextContentScale(Text obj, const char refChar, const float refCharSize) {
    obj->SetContentScale(refChar, refCharSize);
}

NUBUCK_API TransformGizmoMode GetTransformGizmoMode(TransformGizmo obj) {
    return obj->GetTransformMode();
}

NUBUCK_API void SetTransformGizmoMode(TransformGizmo obj, TransformGizmoMode mode) {
    obj->SetTransformMode(mode);
}

NUBUCK_API void SetTransformGizmoPosition(TransformGizmo obj, const M::Vector3& pos) {
    obj->SetPosition(pos);
}

NUBUCK_API void HideTransformGizmo(TransformGizmo obj) {
    obj->Hide();
}

NUBUCK_API void ShowTransformGizmo(TransformGizmo obj) {
    obj->Show();
}

NUBUCK_API TransformGizmo GlobalTransformGizmo() {
    return W::world.GlobalTransformGizmo();
}

NUBUCK_API bool TransformGizmoHandleMouseEvent(
    TransformGizmo obj,
    const EV::MouseEvent& event,
    TransformGizmoMouseInfo& info)
{
    return obj->HandleMouseEvent(event, info);
}

NUBUCK_API void WaitForAnimations() {
    OP::WaitForAnimations();
}

} // namespace NB
