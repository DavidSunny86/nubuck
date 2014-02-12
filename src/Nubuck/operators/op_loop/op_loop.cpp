#include <QMenu>
#include <QAction>

#include <Nubuck\operators\operator_invoker.h>

#include "op_loop.h"

namespace OP {

void Loop::Register(const Nubuck& nb, Invoker& invoker) {
    QAction* action = nb.ui->GetObjectMenu()->addAction("Loop");
    QObject::connect(action, SIGNAL(triggered()), &invoker, SLOT(OnInvoke()));
}

void Loop::DoAction(const EV::Event& event) {
    while(true) {
        printf("OP::LOOP, Doing some action!\n");
        Sleep(100);
    }
}

} // namespace OP