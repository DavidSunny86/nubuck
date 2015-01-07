#include <QObject>
#include <QMenu>

#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_savegeom.h"

static EV::ConcreteEventDef<EV::Arg<std::string*> > ev_save;

namespace OP {

void SaveGeomPanel::Event_Save(const EV::Arg<std::string*>& event) {
    QString filename = QFileDialog::getSaveFileName(
        GetWidget(),
        "Save as...",
        "", // dir,
        "Geometry (*.geom)");
    if(!filename.isNull()) {
        _leFilename->setText(filename);

        EV::Arg<std::string*> event(new std::string(filename.toStdString()));
        SendToOperator(ev_save.Tag(event));
    }
}

SaveGeomPanel::SaveGeomPanel() {
    AddEventHandler(ev_save, this, &SaveGeomPanel::Event_Save);

    QHBoxLayout* layout = new QHBoxLayout;

    layout->addWidget(new QLabel("Filename"));

    _leFilename = new QLineEdit("[...]");
    _leFilename->setReadOnly(true);
    layout->addWidget(_leFilename);

    GetWidget()->setLayout(layout);
}

void SaveGeomPanel::Invoke() {
    _leFilename->setText("[...]");
}

void SaveGeom::Event_Save(const EV::Arg<std::string*>& event) {
    const std::string& filename(*event.value);

    W::SaveGeometryToFile(filename, _mesh);

    delete event.value;
}

SaveGeom::SaveGeom() {
    AddEventHandler(ev_save, this, &SaveGeom::Event_Save);
}

void SaveGeom::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Save as .geom file");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool SaveGeom::Invoke() {
    NB::SetOperatorName("Save (.geom)");

    if(!NB::FirstSelectedMesh()) {
        NB::LogPrintf("no mesh selected.\n");
        return false;
    }

    _mesh = NB::FirstSelectedMesh();

    SendToPanel(ev_save.Tag(NULL));

    return true;
}

} // namespace OP