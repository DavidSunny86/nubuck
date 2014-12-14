#pragma once

#include <Nubuck\nubuck_api.h>
#include <Nubuck\events\events.h>

namespace EV {

struct ResizeEvent : Event {
    EVENT_TYPE(ResizeEvent)

    int width;
    int height;
};

struct MouseEvent : Event {
    EVENT_TYPE(MouseEvent)

    enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
    enum Button { 
        BUTTON_LEFT     = 1,    // == Qt::LeftButton
        BUTTON_RIGHT    = 2,    // == Qt::RightButton
        BUTTON_MIDDLE   = 4     // == Qt::MiddleButton
    };
    enum Modifier {
        MODIFIER_SHIFT  = 0x02000000, // == Qt::ShiftModifier
        MODIFIER_CTRL   = 0x04000000  // == Qt::ControlModifier
    };
    int type, button, mods, delta, x, y;
    int* ret;
};

struct KeyEvent : Event {
    EVENT_TYPE(KeyEvent)

    enum Type { KEY_DOWN = 0, KEY_UP };
    int type;
    int keyCode;
    int nativeScanCode;
    bool autoRepeat;
    enum Modifier {
        MODIFIER_SHIFT = 0x02000000 // == Qt::ShiftModifier
    };
    int mods;
};

} // namespace EV

NUBUCK_API extern EV::ConcreteEventDef<EV::ResizeEvent> ev_resize;
NUBUCK_API extern EV::ConcreteEventDef<EV::MouseEvent>  ev_mouse;
NUBUCK_API extern EV::ConcreteEventDef<EV::KeyEvent>    ev_key;