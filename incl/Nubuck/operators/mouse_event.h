#pragma once

namespace OP {

struct MouseEvent {
    enum Type { MOUSE_DOWN, MOUSE_UP, MOUSE_WHEEL, MOUSE_MOVE };
    enum Button { 
        BUTTON_LEFT     = 1,    // == Qt::LeftButton
        BUTTON_RIGHT    = 2,    // == Qt::RightButton
        BUTTON_MIDDLE   = 4     // == Qt::MiddleButton
    };
    enum Modifier {
        MODIFIER_SHIFT = 0x02000000 // == Qt::ShiftModifier
    };
    Type        type; 
    Button      button;
	int         mods, delta;
    M::Vector2  coords;
};

} // namespace OP