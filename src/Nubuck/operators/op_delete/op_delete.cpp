#include <QMenu>
#include <Nubuck\nubuck.h>
#include <Nubuck\UI\menuitem.h>
#include <Nubuck\operators\operator_invoker.h>
#include <world\world.h>
#include "op_delete.h"

namespace OP {

std::string Delete::PreferredShortcut() const {
    return "D";
}

void Delete::Register(Invoker& invoker) {
    NB::AddMenuItem(NB::ObjectMenu(), "Delete", invoker);
}

bool Delete::Invoke() {
    if(W::editMode_t::OBJECTS != W::world.GetEditMode().GetMode()) {
        NB::LogPrintf("cannot invoke operator 'Delete' when not in OBJECT mode\n");
        return false;
    }

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