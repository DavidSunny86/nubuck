#include <QtGui\QApplication.h>

#include <renderer\glew\glew.h>
#include <QGLWidget>

#include <Nubuck\nubuck.h>
#include <common\common.h>
#include <world\world.h>
#include <world\entities\ent_node\ent_node.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include <world\entities\ent_face\ent_face.h>
#include <world\entities\ent_dummy\ent_dummy.h>
#include <renderer\effects\effect.h>
#include <algdriver\algdriver.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\logwidget\logwidget.h>
#include "nubuck_private.h"

Nubuck nubuck;

QGLFormat FmtAlphaMultisampling(int numSamples) {
	QGLFormat fmt(QGL::AlphaChannel | QGL::SampleBuffers);

	fmt.setSampleBuffers(true);
	fmt.setSamples(numSamples);

	return fmt;
}

int RunNubuck(int argc, char* argv[], algAlloc_t algAlloc) {
    QGLFormat::setDefaultFormat(FmtAlphaMultisampling(4));
    QApplication app(argc, argv);

    common.Init(argc, argv);
    R::CreateDefaultEffects();

    W::world.RegisterEntity<W::ENT_Node>(W::ENT_NODE);
    W::world.RegisterEntity<W::ENT_Polyhedron>(W::ENT_POLYHEDRON);
    W::world.RegisterEntity<W::ENT_Face>(W::ENT_FACE);
    W::world.RegisterEntity<W::ENT_Dummy>(W::ENT_DUMMY);
#ifdef NUBUCK_MT
    W::world.Start();
#endif

    // REMOVEME
    W::Event event;
    event.type = W::ENT_DUMMY;
    event.sem = NULL;
    W::world.Spawn(event);

    nubuck.common   = &common;
    nubuck.world    = &W::world;
    nubuck.log      = UI::LogWidget::Instance();

    ALG::Driver::Instance().SetAlloc(algAlloc);

    UI::MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}