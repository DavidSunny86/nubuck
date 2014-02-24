#pragma once

#include <Nubuck\events\events.h>

BEGIN_EVENT_DEF(Resize)
    int         width;
    int         height;
END_EVENT_DEF

BEGIN_EVENT_DEF(Mouse)
    enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
    enum Button { 
        BUTTON_LEFT     = 1, // == Qt::LeftButton
        BUTTON_RIGHT    = 2, // == Qt::RightButton
        BUTTON_MIDDLE 
    };
    enum Modifier {
        MODIFIER_SHIFT = 0x02000000 // == Qt::ShiftModifier
    };
    int type, button, mods, delta, x, y;
    int* ret;
END_EVENT_DEF

BEGIN_EVENT_DEF(Key)
    enum Type { KEY_DOWN = 0, KEY_UP };
    int type;
    int keyCode;
    bool autoRepeat;
END_EVENT_DEF