#include <QMenu>
#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include "op_delete.h"

namespace OP {

void Delete::Register(Invoker& invoker) {
    QAction* action = NB::ObjectMenu()->addAction("Delete");
    action->setShortcut(QKeySequence("D"));
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

bool Delete::Invoke() {
    NB::SetOperatorName("Delete");

    std::vector<NB::Entity> del;
    NB::Entity ent = NB::FirstSelectedEntity();
    while(ent) {
        del.push_back(ent);
        ent = NB::NextSelectedEntity(ent);
    }

    NB::ClearSelection();

    for(unsigned i = 0; i < del.size(); ++i) {
        NB::DestroyEntity(del[i]);
    }

    return true;
}

} // namespace OP