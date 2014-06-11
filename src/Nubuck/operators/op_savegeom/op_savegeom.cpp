#include <QObject>
#include <QMenu>

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_savegeom.h"

namespace OP {

void SaveGeomPanel::Event_Save(const EV::Event& event) {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save as...",
        "", // dir,
        "Geometry (*.geom)");
    if(!filename.isNull()) {
        _leFilename->setText(filename);

        EV::Params_OP_SaveGeom_Save args = { new std::string(filename.toStdString()) };
        SendToOperator(EV::def_OP_SaveGeom_Save.Create(args));
    }
}

SaveGeomPanel::SaveGeomPanel(QWidget* parent) : OperatorPanel(parent) {
    AddEventHandler(EV::def_OP_SaveGeom_Save, this, &SaveGeomPanel::Event_Save);

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(new QLabel("Filename"));

    _leFilename = new QLineEdit("[...]");
    _leFilename->setReadOnly(true);
    layout->addWidget(_leFilename);

    setLayout(layout);
}

void SaveGeomPanel::Invoke() {
    _leFilename->setText("[...]");
}

void SaveGeom::Event_Save(const EV::Event& event) {
    const EV::Params_OP_SaveGeom_Save& args = EV::def_OP_SaveGeom_Save.GetArgs(event);
    const std::string& filename(*args.filename);

    W::SaveGeometryToFile(filename, _geom);

    delete args.filename;
}

SaveGeom::SaveGeom() {
    AddEventHandler(EV::def_OP_SaveGeom_Save, this, &SaveGeom::Event_Save);
}

void SaveGeom::Register(const Nubuck& nb, Invoker& invoker) {
    _nb = nb;

    QAction* action = _nb.ui->GetObjectMenu()->addAction("Save as .geom file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool SaveGeom::Invoke() {
    _nb.ui->SetOperatorName("Save (.geom)");

    ISelection* sel = _nb.world->GetSelection();
    std::vector<IGeometry*> geomSel = sel->GetList();
    if(geomSel.empty()) {
        _nb.log->printf("no geometry selected.\n");
        return false;
    }

    _geom = geomSel[0];

    EV::Params_OP_SaveGeom_Save args = { NULL };
    SendToPanel(EV::def_OP_SaveGeom_Save.Create(args));

    return true;
}

} // namespace OP