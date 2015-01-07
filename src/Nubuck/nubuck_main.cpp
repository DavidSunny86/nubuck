#include <QtGui\QApplication.h>
#include <QTimer>

#include <renderer\glew\glew.h>
#include <QGLWidget>

#include <Nubuck\nubuck.h>
#include <Nubuck\common\common.h>
#include <Nubuck\animation\animator.h>
#include <common\config\config.h>
#include <common\commands.h>
#include <system\opengl\opengl.h>
#include <world\world.h>
#include <world\entities\ent_text\ent_text.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>
#include <renderer\effects\effect.h>
#include <renderer\effects\statedesc_gen\statedesc_gen.h>
#include <UI\glwidget\glwidget.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\logwidget\logwidget.h>
#include <UI\userinterface.h>
#include <UI\nb_widgets.h>
#include <mainloop\mainloop.h>
#include "nubuck_private.h"

// REMOVEME
#include <Nubuck\operators\operator_invoker.h>
#include <operators\operators.h>
#include <operators\op_loop\op_loop.h>
#include <operators\op_gen_randompoints\op_gen_randompoints.h>
#include <operators\op_gen_platonic_solids\op_gen_platonic_solids.h>
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

static void InitializeRenderer() {
    // somewhat HACKY. create dummy glwidget and rc to init renderer
    UI::GLWidget* dummyGL = new UI::GLWidget;
    dummyGL->Use();
    R::theRenderer.Init();
    delete dummyGL;
}

void TestPageAlloc();

int RunNubuck(
    int                         argc,
    char*                       argv[],
    createOperatorFunc_t        createOperatorFunc,
    createOperatorPanelFunc_t   createOperatorPanelFunc)
{
#ifdef _DEBUG
    TestPageAlloc();
#endif

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

    SYS::InitializeGLExtensions();
    InitializeRenderer();

    OP::g_operators.Init();
    W::world.Init();

    g_ui.Init();
    W::world.GetEditMode().AddObserver(&g_ui);
    W::world.GetEditMode().AddObserver(&W::world);
    W::world.GetEditMode().AddObserver(&OP::g_operators);
    QObject::connect(qApp, SIGNAL(aboutToQuit()), &g_ui, SLOT(OnQuit()));

    MainLoop mainLoop;
    mainLoop.Enter();

    // register commands
    COM::CMD::RegisterCommand("lsvars", "list config variables", COM::CMD_ListVariables);
    COM::CMD::RegisterCommand("set", "set config variable", COM::CMD_SetVariable);

    // REMOVEME
	OP::g_operators.Register(new OP::TranslatePanel, new OP::Translate);
    OP::g_operators.Register(new OP::LoopPanel, new OP::Loop);
    OP::g_operators.Register(new OP::GEN::RandomPointsPanel, new OP::GEN::RandomPoints);
    OP::g_operators.Register(new OP::PlatonicSolidsPanel, new OP::PlatonicSolids);
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

    // register user operator
    OP::Operator*       userOperator = NULL;
    OP::OperatorPanel*  userOperatorPanel = NULL;
    if(createOperatorFunc) {
        userOperator = createOperatorFunc();
        if(createOperatorPanelFunc) {
            userOperatorPanel = createOperatorPanelFunc();
        }
        OP::g_operators.Register(userOperatorPanel, userOperator);
    }

    g_ui.GetMainWindow().show();
    const int ret = app.exec();

    // cleanup
    R::theRenderer.Destroy();

    return ret;
}