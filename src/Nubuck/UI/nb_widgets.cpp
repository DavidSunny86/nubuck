#include <Nubuck\events\core_events.h>
#include <operators\operators.h>
#include "nb_widgets.h"

void NBW_Button::OnClicked() {
    OP::SendToOperator(ev_buttonClicked.Tag(EV::Event(), _id));
}

NBW_Button::NBW_Button(unsigned id, const char* name) 
    : QPushButton(QString(name))
    , _id(id)
{
    // i wonder if this is the canonical way to do this...
    connect(this, SIGNAL(clicked()), this, SLOT(OnClicked()));
}

void NBW_CheckBox::OnToggled(bool isChecked) {
    OP::SendToOperator(ev_checkBoxToggled.Tag(isChecked, _id));
}

NBW_CheckBox::NBW_CheckBox(unsigned id, const char* name)
    : QCheckBox(QString(name))
    , _id(id)
{
    connect(this, SIGNAL(toggled(bool)), this, SLOT(OnToggled(bool)));
}