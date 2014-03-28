#pragma once

#include <Nubuck\events\events.h>

class UserInterface : public EV::EventHandler<> {
private:
    void Event_EditModeChanged(const EV::Event& event);
public:
    void Init();

    DECL_HANDLE_EVENTS(UserInterface);
};

extern UserInterface g_ui;