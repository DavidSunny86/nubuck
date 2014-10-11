#include <QtGui\QApplication.h>
#include <QTimer>

#include <renderer\glew\glew.h>
#include <QGLWidget>

#include <Nubuck\nubuck.h>
#include <Nubuck\common\common.h>
#include <Nubuck\animation\animator.h>
#include <common\config\config.h>
#include <common\commands.h>
#include <world\world.h>
#include <world\entities\ent_text\ent_text.h>
#include <renderer\effects\effect.h>
#include <renderer\effects\statedesc_gen\statedesc_gen.h>
#include <algdriver\algdriver.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\logwidget\logwidget.h>
#include <UI\userinterface.h>
#include <mainloop\mainloop.h>
#include "nubuck_private.h"

// REMOVEME
#include <operators\operators.h>
#include <operators\op_loop\op_loop.h>
#include <operators\op_gen_randompoints\op_gen_randompoints.h>
#include <operators\op_gen_merge\op_gen_merge.h>
#include <operators\op_gen_windows\op_gen_windows.h>
#include <operators\op_loadobj\op_loadobj.h>
#include <operators\op_loadgeom\op_loadgeom.h>
#include <operators\op_chull\op_chull.h>
#include <operators\op_fchull\op_fchull.h>
#include <operators\op_delaunay3d\op_delaunay3d.h>
#include <operators\op_translate\op_translate.h>
#include <operators\op_join\op_join.h>
#include <operators\op_delete\op_delete.h>
#include <operators\op_savegeom\op_savegeom.h>
#include <operators\op_merge_vertices\op_merge_vertices.h>

bool g_showRenderViewControls = true;

void ToggleRenderViewControls() {
    g_showRenderViewControls = !g_showRenderViewControls;
}

void NubuckImpl::log_printf(const char* format, ...) {
    static char buffer[2048];

    memset(buffer, 0, sizeof(buffer));
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    UI::logWidget().printf(format);
}

QMenu* NubuckImpl::scene_menu() {
    return g_ui.GetMainWindow().GetSceneMenu();
}

QMenu* NubuckImpl::object_menu() {
    return g_ui.GetMainWindow().GetObjectMenu();
}

QMenu* NubuckImpl::algorithm_menu() {
    return g_ui.GetMainWindow().GetAlgorithmMenu();
}

QMenu* NubuckImpl::vertex_menu() {
    return g_ui.GetMainWindow().GetVertexMenu();
}

void NubuckImpl::set_operator_name(const char* name) {
    return g_ui.GetMainWindow().SetOperatorName(name);
}

void NubuckImpl::set_operator_panel(QWidget* panel) {
    return g_ui.GetMainWindow().SetOperatorPanel(panel);
}

void NubuckImpl::destroy(const nb::entity obj) {
    obj->Destroy();
}

nb::geometry NubuckImpl::create_geometry() {
    return W::world.CreateGeometry();
}

void NubuckImpl::destroy_geometry(const nb::geometry obj) {
    obj->Destroy();
}

nb::text NubuckImpl::create_text() {
    return W::world.CreateText();
}

void NubuckImpl::clear_selection() {
    W::world.ClearSelection();
}

void NubuckImpl::select_geometry(SelectMode mode, const nb::geometry obj) {
    if(SELECT_MODE_NEW == mode) W::world.Select_New(obj);
    else W::world.Select_Add(obj);
}

void NubuckImpl::select(SelectMode mode, const nb::entity obj) {
    if(SELECT_MODE_NEW == mode) W::world.Select_New(obj);
    else W::world.Select_Add(obj);
}

void NubuckImpl::select_vertex(SelectMode mode, const nb::geometry obj, const leda::node vert) {
    if(SELECT_MODE_NEW == mode) W::world.SelectVertex_New(obj, vert);
    else W::world.SelectVertex_Add(obj, vert);
}

std::vector<nb::geometry> NubuckImpl::selected_geometry() {
    return W::world.SelectedGeometry();
}

M::Vector3 NubuckImpl::global_center_of_selection() {
    return W::world.GlobalCenterOfSelection();
}

M::Vector3 NubuckImpl::position(nb::entity obj) {
    return obj->GetPosition();
}

nb::EntityType::Enum NubuckImpl::type_of(nb::entity obj) {
    return nb::EntityType::Enum(obj->GetType()); // types are compatible
}

nb::geometry NubuckImpl::to_geometry(nb::entity obj) {
    assert(W::EntityType::ENT_GEOMETRY == obj->GetType());
    return static_cast<W::ENT_Geometry*>(obj); // ugly but safe downcast
}

nb::text NubuckImpl::to_text(nb::entity obj) {
    assert(W::EntityType::ENT_TEXT == obj->GetType());
    return static_cast<W::ENT_Text*>(obj); // ugly but safe downcast
}

nb::entity NubuckImpl::first_selected_entity() {
    return W::world.FirstSelectedEntity();
}

nb::entity NubuckImpl::next_selected_entity(nb::entity obj) {
    return W::world.NextSelectedEntity(obj);
}

void NubuckImpl::set_position(nb::entity obj, const M::Vector3& pos) {
    obj->SetPosition(pos);
}

const std::string& NubuckImpl::geometry_name(const nb::geometry obj) {
    return obj->GetName();
}

M::Vector3 NubuckImpl::geometry_position(const nb::geometry obj) {
    return obj->GetPosition();
}

nb::geometry NubuckImpl::first_selected_geometry() {
    W::Entity* ent = W::world.FirstSelectedEntity();
    while(ent) {
        if(W::EntityType::ENT_GEOMETRY == ent->GetType()) {
            return static_cast<W::ENT_Geometry*>(ent); // ugly but safe downcast
        }
        ent = ent->selectionLink.next;
    }
    return NULL;
}

nb::geometry NubuckImpl::next_selected_geometry(nb::geometry obj) {
    W::Entity* ent = obj->selectionLink.next;
    while(ent) {
        if(W::EntityType::ENT_GEOMETRY == ent->GetType()) {
            return static_cast<W::ENT_Geometry*>(ent); // ugly but safe downcast
        }
        ent = ent->selectionLink.next;
    }
    return NULL;
}

leda::nb::RatPolyMesh& NubuckImpl::poly_mesh(const nb::geometry obj) {
    return obj->GetRatPolyMesh();
}

void NubuckImpl::set_geometry_name(const nb::geometry obj, const std::string& name) {
    obj->SetName(name);
}

void NubuckImpl::apply_geometry_transformation(const nb::geometry obj) {
    obj->ApplyTransformation();
}

void NubuckImpl::set_geometry_position(const nb::geometry obj, const M::Vector3& position) {
    obj->SetPosition(position);
}

void NubuckImpl::set_geometry_scale(const nb::geometry obj, const M::Vector3& scale) {
    obj->SetScale(scale);
}

void NubuckImpl::hide_geometry_outline(const nb::geometry obj) {
    obj->HideOutline();
}

void NubuckImpl::hide_geometry(const nb::geometry obj) {
    obj->Hide();
}

void NubuckImpl::show_geometry(const nb::geometry obj) {
    obj->Show();
}

void NubuckImpl::set_geometry_solid(const nb::geometry obj, bool solid) {
    obj->SetSolid(solid);
}

void NubuckImpl::set_geometry_render_mode(const nb::geometry obj, int flags) {
    obj->SetRenderMode(flags);
}

void NubuckImpl::set_geometry_render_layer(const nb::geometry obj, unsigned layer) {
    obj->SetRenderLayer(layer);
}

void NubuckImpl::set_geometry_shading_mode(const nb::geometry obj, ShadingMode::Enum mode) {
    obj->SetShadingMode(mode);
}

const M::Vector2& NubuckImpl::text_content_size(const nb::text obj) const {
    return obj->GetContentSize();
}

void NubuckImpl::set_text_position(const nb::text obj, const M::Vector3& position) {
    obj->SetPosition(position);
}

void NubuckImpl::set_text_content(const nb::text obj, const std::string& content) {
    obj->SetContent(content);
}

void NubuckImpl::set_text_content_scale(const nb::text obj, const char refChar, const float refCharSize) {
    obj->SetContentScale(refChar, refCharSize);
}

void NubuckImpl::wait_for_animations() {
    OP::WaitForAnimations();
}

NubuckImpl g_nubuck;

Nubuck& nubuck() { return g_nubuck; }

namespace {

    std::string ReadFile(const std::string& filename) {
        std::string text;

        std::ifstream file(filename.c_str());
        if(file.is_open()) {
            std::string line;
            while(file.good()) {
                std::getline(file, line);
                text += line;
            }
        }

        return text;
    }

} // unnamed namespace

QGLFormat FmtAlphaMultisampling(int numSamples) {
	QGLFormat fmt(QGL::AlphaChannel | QGL::SampleBuffers);

    if(0 < numSamples) {
        fmt.setSampleBuffers(true);
        fmt.setSamples(numSamples);
    }

	return fmt;
}

int RunNubuck(int argc, char* argv[], algAlloc_t algAlloc) {
    QGLFormat::setDefaultFormat(FmtAlphaMultisampling(0));
    QApplication app(argc, argv);

    COM::Config::Instance().DumpVariables();

    common.Init(argc, argv);
    R::CreateDefaultEffects();

    bool useStylesheet = true;

    unsigned i = 0;
    while(i < argc) {
        if(!strcmp("--no-stylesheet", argv[i])) {
            useStylesheet = false;
        }
        if(!strcmp("--genstatedesc", argv[i])) {
            common.printf("INFO - generating statedesc.\n");
            const std::string inname = common.BaseDir() + "stategen_test\\state.h";
            const std::string outname = common.BaseDir() + "stategen_test\\statedesc.cpp";
            if(STG_Parse(inname.c_str(), outname.c_str())) {
                common.printf("ERROR - STG_Parse(%s, %s) failed.\n", inname.c_str(), outname.c_str());
            }
        }
        i++;
    }

    // read stylesheet
    if(useStylesheet) {
        std::string stylesheetFilename = common.BaseDir() + "nubuck.stylesheet";
        common.printf("INFO - reading stylesheet: %s\n", stylesheetFilename.c_str());
        const std::string stylesheet = ReadFile(stylesheetFilename.c_str());
        if(stylesheet.empty()) {
            common.printf("ERROR - unable to read stylesheet %s\n", stylesheetFilename.c_str());
            Crash();
        } else {
            QString styleSheet(QString::fromStdString(stylesheet));
            app.setStyleSheet(styleSheet);
        }
    }

    common.printf("INFO - Nubuck compiled with Qt version '%s'\n", QT_VERSION_STR);

    g_ui.Init();
    W::world.GetEditMode().AddObserver(&g_ui);
    W::world.GetEditMode().AddObserver(&W::world);
    W::world.GetEditMode().AddObserver(&OP::g_operators);
    QObject::connect(qApp, SIGNAL(aboutToQuit()), &g_ui, SLOT(OnQuit()));

    MainLoop mainLoop;
    mainLoop.Enter();

    ALG::gs_algorithm.SetAlloc(algAlloc);

    // register commands
    COM::CMD::RegisterCommand("lsvars", "list config variables", COM::CMD_ListVariables);
    COM::CMD::RegisterCommand("set", "set config variable", COM::CMD_SetVariable);

    // REMOVEME
	OP::g_operators.Register(new OP::TranslatePanel, new OP::Translate);
    OP::g_operators.Register(new OP::LoopPanel, new OP::Loop);
    OP::g_operators.Register(new OP::GEN::RandomPointsPanel, new OP::GEN::RandomPoints);
    OP::g_operators.Register(new OP::GEN::MergePanel, new OP::GEN::Merge);
    OP::g_operators.Register(new OP::GEN::WindowsPanel, new OP::GEN::Windows);
	OP::g_operators.Register(new OP::LoadOBJPanel, new OP::LoadOBJ);
    OP::g_operators.Register(new OP::LoadGeomPanel, new OP::LoadGeom);
	OP::g_operators.Register(new OP::ConvexHullPanel, new OP::ConvexHull);
    OP::g_operators.Register(new OP::FlipClipPanel, new OP::FlipClip);
    OP::g_operators.Register(new OP::Delaunay3DPanel, new OP::Delaunay3D);
	OP::g_operators.Register(new OP::JoinPanel, new OP::Join);
	OP::g_operators.Register(new OP::DeletePanel, new OP::Delete);
    OP::g_operators.Register(new OP::SaveGeomPanel, new OP::SaveGeom);
    OP::g_operators.Register(new OP::MergeVerticesPanel, new OP::MergeVertices);
	OP::g_operators.OnInvokeOperator(0); // call OP::Translate
    OP::LoadOperators();

    g_ui.GetMainWindow().show();
    return app.exec();
}