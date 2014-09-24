#include <QMenu>
#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include "op_delete.h"

namespace OP {

void Delete::Register(const Nubuck& nb, Invoker& invoker) {
    QAction* action = nubuck().object_menu()->addAction("Delete");
    action->setShortcut(QKeySequence("D"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Delete::Invoke() {
    nubuck().set_operator_name("Delete");

    std::vector<nb::geometry> geomList = nubuck().selected_geometry();
    for(unsigned i = 0; i < geomList.size(); ++i) {
        nubuck().destroy_geometry(geomList[i]);
    }
    nubuck().clear_selection();

    return true;
}

} // namespace OP