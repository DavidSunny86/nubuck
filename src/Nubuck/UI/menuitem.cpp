#include <iostream>

#include <QShortcutEvent>

#include <Nubuck\events\core_events.h>
#include <Nubuck\UI\menuitem.h>
#include <operators\operators.h>

namespace UI {

bool MenuItem::event(QEvent* ev) {
    if(QEvent::Shortcut == ev->type()) {
        // ignore shortcut and generate keyevent instead

        QShortcutEvent* shortcut = static_cast<QShortcutEvent*>(ev);

        const QKeySequence& ks = shortcut->key();
        COM_assert(1 == ks.count()); // note that modifiers do not count as individual keys

        // extract modifiers
        const Qt::KeyboardModifiers mods(ks[0] & (Qt::SHIFT | Qt::CTRL | Qt::ALT));

        // extract key as ks minus mods. key must match [0-9A-Z]
        const int key = ks[0] & ~mods;
        COM_assert(
            (Qt::Key_0 <= key && key <= Qt::Key_9) ||
            (Qt::Key_A <= key && key <= Qt::Key_Z));
        
        QKeyEvent qt_event(QEvent::KeyPress, key, mods);

        EV::KeyEvent nb_event;
        nb_event.type = EV::KeyEvent::KEY_DOWN;
        nb_event.keyCode = qt_event.key();
        nb_event.nativeScanCode = qt_event.nativeScanCode();
        nb_event.autoRepeat = qt_event.isAutoRepeat();
        nb_event.mods = qt_event.modifiers();

        std::cout << "MenuItem::event(), generating event " << nb_event << std::endl;

        OP::g_operators.InvokeAction(ev_key.Tag(nb_event),
            OP::Operators::InvokationMode::DROP_WHEN_BUSY);

        return true;
    } else {
        return QAction::event(ev);
    }
}

MenuItem::MenuItem(QObject* parent) : QAction(parent) { }

MenuItem::MenuItem(const QString& text, QObject* parent) : QAction(text, parent) { }

} // namespace UI