#include <QFileDialog>
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>

#include <world\entities\ent_geometry\ent_geometry.h>
#include <Nubuck\polymesh.h>
#include <Nubuck\operators\operator_invoker.h>
#include "op_loadobj.h"

namespace OP {

void LoadOBJ::OnChooseFilename() {
	QString filename = QFileDialog::getOpenFileName(
        _panel,
        "Choose a .obj file",
		QDir::currentPath(),
        tr("Models (*.obj)"));
	if(!filename.isNull()) {
		_ui.lneFilename->setText(filename);

        if(_geom) _geom->Destroy();

        _geom = (W::ENT_Geometry*)_nb.world->CreateGeometry();
        _geom->SetRenderMode(IGeometry::RenderMode::NODES | IGeometry::RenderMode::EDGES | IGeometry::RenderMode::FACES);
        leda::nb::RatPolyMesh& mesh = _geom->GetRatPolyMesh();
		mesh.FromObj(filename.toAscii());
        _geom->Update();

        _nb.world->SelectGeometry(_geom);
	}
}

void LoadOBJ::BuildPanel() {
    _panel = new QWidget();
	_ui.setupUi(_panel);
	_nb.ui->SetOperatorPanel(_panel);

	QObject::connect(_ui.btnChooseFile, SIGNAL(clicked()), this, SLOT(OnChooseFilename()));
}

void LoadOBJ::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetSceneMenu()->addAction("Load .obj file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void LoadOBJ::Invoke() {
    printf("LoadOBJ::Invoke\n");

    _geom = NULL;
    BuildPanel();
}

} // namespace OP