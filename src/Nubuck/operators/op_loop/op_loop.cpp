#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>

#include "op_loop.h"

namespace OP {

void Loop::Event_OP_Loop_Start(const EV::Event& event) {
    for(unsigned i = 0; i < 50; ++i) {
        printf("OP::LOOP %8d, Doing some action!\n", i);
        Sleep(100);
    }
}

Loop::Loop() {
    AddEventHandler(EV::def_OP_Loop_Start, this, &Loop::Event_OP_Loop_Start);
}

void Loop::Register(const Nubuck& nb, Invoker& invoker) {
    QAction* action = nb.ui->GetObjectMenu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

} // namespace OP