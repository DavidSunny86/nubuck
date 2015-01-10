#pragma once

#include <string>
#include <vector>

#include <Windows.h> // defines MAX_INT
#include <LEDA\geo\d3_rat_point.h>
#include <LEDA\graph\graph.h>

#include <Nubuck\nubuck_api.h>
#include <Nubuck\polymesh_fwd.h>
#include <Nubuck\math\vector3.h>

/*
when to use 'get' prefix for method names:

a method that returns a property of an instance should be prefixed with 'get'.
examples: 
Vector3 GetEntityPosition(Entity entity);
Vector3 GlobalCenterOfSelection(); // no instance as argument
*/

class QMenu;
class QWidget;
class QBoxLayout;

namespace M {

struct Vector2;

} // namespace M

namespace R {

struct Color;

} // namespace R

namespace EV {

struct MouseEvent;

} // namespace EV

namespace W {

class Entity;
class ENT_Geometry;
class ENT_Text;
class ENT_TransformGizmo;

} // namespace W

namespace OP {

class Operator;
class OperatorPanel;
class Invoker;

} // namespace OP

class NBW_Button;

namespace NB {

typedef W::Entity*              Entity;
typedef W::ENT_Geometry*        Mesh;
typedef W::ENT_Text*            Text;
typedef W::ENT_TransformGizmo*  TransformGizmo;

enum EntityType {
    ET_GEOMETRY = 0,
    ET_TEXT,
    ET_TRANSFORM_GIZMO
};

typedef QBoxLayout* BoxLayout;
typedef QWidget*    Widget;
typedef NBW_Button* Button;

enum AxisFlags {
    AF_X = 1,
    AF_Y = 2,
    AF_Z = 4,

    AF_XYZ = AF_X | AF_Y | AF_Z
};

// logfile
NUBUCK_API void LogPrintf(const char* format, ...);

// user interface
NUBUCK_API QMenu*  SceneMenu();
NUBUCK_API QMenu*  ObjectMenu();
NUBUCK_API QMenu*  AlgorithmMenu();
NUBUCK_API QMenu*  VertexMenu();
NUBUCK_API void    AddMenuItem(QMenu* menu, const char* name, OP::Invoker& invoker);
NUBUCK_API void    SetOperatorName(const char* name);
NUBUCK_API void    SetOperatorPanel(QWidget* panel);

NUBUCK_API Widget      CastToWidget(Button button);
NUBUCK_API BoxLayout   CreateHorizontalBoxLayout();
NUBUCK_API BoxLayout   CreateVerticalBoxLayout();
NUBUCK_API Button      CreateButton(unsigned id, const char* text);
NUBUCK_API void        AddWidgetToBox(BoxLayout box, Widget widget);

// world
NUBUCK_API void            DestroyEntity(Entity entity);
NUBUCK_API Mesh            CreateMesh();
NUBUCK_API void            DestroyMesh(Mesh mesh);
NUBUCK_API Text            CreateText();
NUBUCK_API TransformGizmo  CreateTransformGizmo();

// selection
enum SelectMode {
    SM_NEW,
    SM_ADD
};

NUBUCK_API void ClearSelection();
NUBUCK_API void SelectMesh(SelectMode mode, Mesh mesh);
NUBUCK_API void SelectEntity(SelectMode mode, Entity entity);
NUBUCK_API void SelectVertex(SelectMode mode, Mesh mesh, const leda::node vertex);

NUBUCK_API M::Vector3 GlobalCenterOfSelection();

// entities
NUBUCK_API M::Vector3 GetEntityPosition(Entity entity);

NUBUCK_API EntityType  GetType(Entity entity);
NUBUCK_API Mesh        CastToMesh(Entity entity);
NUBUCK_API Text        CastToText(Entity entity);

NUBUCK_API Entity FirstSelectedEntity();
NUBUCK_API Entity NextSelectedEntity(Entity entity);

NUBUCK_API void SetEntityPosition(Entity entity, const M::Vector3& pos);

// geometry
enum RenderMode {
    RM_FACES       = (1 << 0),
    RM_NODES       = (1 << 1),
    RM_EDGES       = (1 << 2),
    RM_ALL         = RM_NODES | RM_EDGES | RM_FACES
};

enum ShadingMode {
    SM_NICE = 0,
    SM_FAST,
    SM_LINES,
    SM_NICE_BILLBOARDS,

    NUM_SHADING_MODES
};

enum Pattern {
    PATTERN_NONE,
    PATTERN_CHECKER,
    PATTERN_DOTS,
    PATTERN_LINES
};

// mesh
NUBUCK_API const std::string&  GetMeshName(Mesh mesh);
NUBUCK_API M::Vector3          GetMeshPosition(Mesh mesh);

NUBUCK_API Mesh FirstSelectedMesh();
NUBUCK_API Mesh NextSelectedMesh(Mesh mesh);

NUBUCK_API void                SetGraph(Mesh mesh, const leda::NbGraph& graph);
NUBUCK_API leda::NbGraph&      GetGraph(Mesh mesh);

NUBUCK_API void SetMeshName(Mesh mesh, const std::string& name);

NUBUCK_API void ApplyMeshTransformation(Mesh mesh);

NUBUCK_API void SetMeshPosition(Mesh mesh, const M::Vector3& position);
NUBUCK_API void SetMeshScale(Mesh mesh, const M::Vector3& scale);

NUBUCK_API void HideMeshOutline(Mesh mesh);

NUBUCK_API void HideMesh(Mesh mesh);
NUBUCK_API void ShowMesh(Mesh mesh);

NUBUCK_API void SetMeshSolid(Mesh mesh, bool solid);
NUBUCK_API void SetMeshRenderMode(Mesh mesh, int renderModeFlags);
NUBUCK_API void SetMeshShadingMode(Mesh mesh, ShadingMode shadingMode);
NUBUCK_API void SetMeshPattern(Mesh mesh, Pattern pattern);
NUBUCK_API void SetMeshPatternColor(Mesh mesh, const R::Color& color);

// text
NUBUCK_API const M::Vector2& GetTextContentSize(Text text);

NUBUCK_API void SetTextPosition(Text text, const M::Vector3& pos);
NUBUCK_API void SetTextContent(Text text, const std::string& content);
NUBUCK_API void SetTextContentScale(Text text, const char refChar, const float refCharSize);

// transform gizmo
enum TransformGizmoMode {
    TGM_TRANSLATE = 0,
    TGM_SCALE,
    TGM_ROTATE,
    TGM_NUM_MODES
};

enum TransformGizmoAction {
    TGA_NO_ACTION,
    TGA_BEGIN_DRAGGING,
    TGA_END_DRAGGING,
    TGA_DRAGGING
};

enum Axis {
    AXIS_X, AXIS_Y, AXIS_Z, INVALID_AXIS
};

struct TransformGizmoMouseInfo {
    TransformGizmoAction    action;
    Axis                    axis;
    float                   value; // either translation or scale
};

NUBUCK_API TransformGizmoMode GetTransformGizmoMode(TransformGizmo tfgizmo);

NUBUCK_API void SetTransformGizmoMode(TransformGizmo tfgizmo, TransformGizmoMode mode);

NUBUCK_API void SetTransformGizmoPosition(TransformGizmo tfgizmo, const M::Vector3& pos);

NUBUCK_API void HideTransformGizmo(TransformGizmo tfgizmo);
NUBUCK_API void ShowTransformGizmo(TransformGizmo tfgizmo);

NUBUCK_API TransformGizmo GlobalTransformGizmo();

NUBUCK_API bool TransformGizmoHandleMouseEvent(
    TransformGizmo tfgizmo,
    const EV::MouseEvent& event,
    TransformGizmoMouseInfo& info);

// animation
NUBUCK_API void WaitForAnimations();

} // namespace NB

typedef OP::Operator*       (*createOperatorFunc_t)();
typedef OP::OperatorPanel*  (*createOperatorPanelFunc_t)();

template<typename TYPE> OP::Operator*       CreateOperator() { return new TYPE; }
template<typename TYPE> OP::OperatorPanel*  CreateOperatorPanel() { return new TYPE; }

// use the macros if you're afraid of template syntax
#define CREATE_OPERATOR(type)        CreateOperator<type>
#define CREATE_OPERATOR_PANEL(type)  CreateOperatorPanel<type>

/*
main entry point, returns exit code.
if panel creation function is NULL an empty panel is provided.
the operator creation function may be NULL, in which case Nubuck
is started with build-in operators only.
typical usage:
RunNubuck(argc, argv, CreateOperator<MyOperator>, CreateOperatorPanel<MyOperatorPanel>);
*/
NUBUCK_API int RunNubuck(
    int                         argc,
    char*                       argv[],
    createOperatorFunc_t        createOperatorFunc,
    createOperatorPanelFunc_t   createOperatorPanelFunc);


