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

    std::vector<nb::entity> del;
    nb::entity ent = nubuck().first_selected_entity();
    while(ent) {
        del.push_back(ent);
        ent = nubuck().next_selected_entity(ent);
    }

    nubuck().clear_selection();

    for(unsigned i = 0; i < del.size(); ++i) {
        nubuck().destroy(del[i]);
    }

    return true;
}

} // namespace OP