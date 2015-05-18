#include <Nubuck\events\events.h>
#include <Nubuck\events\core_events.h>
#include <world\world_events.h>
#include <UI\outliner\outliner.h>
#include <operators\operator_events.h>
#include <world\entities\ent_geometry\ent_geometry_events.h>

eventID_t eventID_Cnt = 0;

eventID_t EV_GetNextID() {
    return ++eventID_Cnt;
}

// core events
NUBUCK_API EV::ConcreteEventDef<EV::ResizeEvent> ev_resize;
NUBUCK_API EV::ConcreteEventDef<EV::MouseEvent>  ev_mouse;
NUBUCK_API EV::ConcreteEventDef<EV::KeyEvent>    ev_key;
NUBUCK_API EV::ConcreteEventDef<EV::Event>       ev_buttonClicked;
NUBUCK_API EV::ConcreteEventDef<EV::Arg<bool> >  ev_checkBoxToggled;

NUBUCK_API EV::ConcreteEventDef<EV::Event>       ev_op_requestFinish;

NUBUCK_API EV::ConcreteEventDef<EV::ShowQuestionBox> ev_ui_showQuestionBox;

NUBUCK_API EV::ConcreteEventDef<EV::Usr_SelectEntity>   ev_usr_selectEntity;
NUBUCK_API EV::ConcreteEventDef<EV::Arg<int> >          ev_usr_changeEditMode;

 // world events
EV::ConcreteEventDef<EV::Event>              ev_w_apocalypse;
EV::ConcreteEventDef<EV::Arg<W::Entity*> >   ev_w_linkEntity;
EV::ConcreteEventDef<EV::Arg<unsigned> >     ev_w_destroyEntity;
EV::ConcreteEventDef<EV::Event>              ev_w_rebuildAll;
EV::ConcreteEventDef<EV::Event>              ev_w_selectionChanged;
EV::ConcreteEventDef<EV::Event>              ev_w_cameraChanged;
EV::ConcreteEventDef<EV::Arg<int> >          ev_w_editModeChanged;
EV::ConcreteEventDef<EV::Event>              ev_w_meshChanged;

// outliner events
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_createView;
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_hide;
EV::ConcreteEventDef<EV::Args2<outlinerItem_t, QString*> >   ev_outl_setName;
EV::ConcreteEventDef<EV::Arg<outlinerItem_t> >               ev_outl_delete;

// operator events
EV::ConcreteEventDef<EV::Event>                         ev_op_actionFinished;
EV::ConcreteEventDef<EV::Args2<OP::Operator*, bool> >   ev_op_setOperator;
EV::ConcreteEventDef<EV::Event>                         ev_op_showConfirmationDialog;

// geometry events
EV::ConcreteEventDef<EV::Arg<bool> >     ev_geom_showVertexLabels;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_setVertexLabelSize;
EV::ConcreteEventDef<EV::Arg<bool> >     ev_geom_xrayVertexLabels;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_vertexScaleChanged;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_edgeScaleChanged;
EV::ConcreteEventDef<EV::Arg<R::Color> > ev_geom_edgeColorChanged;
EV::ConcreteEventDef<EV::Arg<float> >    ev_geom_transparencyChanged;
EV::ConcreteEventDef<RenderModeEvent>    ev_geom_renderModeChanged;
EV::ConcreteEventDef<EdgeShadingEvent>   ev_geom_edgeShadingChanged;

// output

namespace EV {

std::ostream& operator<<(std::ostream& stream, const MouseEvent& mouseEvent) {
    stream << "MouseEvent {";

    switch(mouseEvent.type) {
    case MouseEvent::MOUSE_DOWN:
        stream << " MOUSE_DOWN";
        break;
    case MouseEvent::MOUSE_UP:
        stream << " MOUSE_UP";
        break;
    case MouseEvent::MOUSE_MOVE:
        stream << " MOUSE_MOVE";
        break;
    case MouseEvent::MOUSE_WHEEL:
        stream << " MOUSE_WHEEL";
        break;
    default:
        COM_assert(0 && "unknown mouse event type");
    }

    switch(mouseEvent.button) {
    case MouseEvent::BUTTON_NONE:
        stream << "BUTTON_NONE";
        break;
    case MouseEvent::BUTTON_LEFT:
        stream << " BUTTON_LEFT";
        break;
    case MouseEvent::BUTTON_RIGHT:
        stream << " BUTTON_RIGHT";
        break;
    case MouseEvent::BUTTON_MIDDLE:
        stream << " BUTTON_MIDDLE";
        break;
    default:
        common.printf("unknown mouse event button %d\n", mouseEvent.button);
        Crash();
    };

    stream << " mods = (";
    if(MouseEvent::MODIFIER_CTRL & mouseEvent.mods) stream << " MODIFIER_CTRL";
    if(MouseEvent::MODIFIER_SHIFT & mouseEvent.mods) stream << " MODIFIER_SHIFT";
    stream << ")";

    stream
        << " x = " << mouseEvent.x
        << ", y = " << mouseEvent.y
        << ", delta = " << mouseEvent.delta;

    stream << " }";

    return stream;
}

std::ostream& operator<<(std::ostream& stream, const KeyEvent& keyEvent) {
    stream << "KeyEvent {";

    switch(keyEvent.type) {
    case KeyEvent::KEY_UP:
        stream << " KEY_UP";
        break;
    case KeyEvent::KEY_DOWN:
        stream << " KEY_DOWN";
        break;
    }

    stream << " ";
    if(KeyEvent::MODIFIER_SHIFT & keyEvent.mods) stream << "SHIFT+";
    if(KeyEvent::MODIFIER_CTRL & keyEvent.mods) stream << "CTRL+";
    if(KeyEvent::MODIFIER_ALT & keyEvent.mods) stream << "ALT+";

    if(Qt::Key_Space <= keyEvent.keyCode && keyEvent.keyCode <= Qt::Key_AsciiTilde) {
        stream << static_cast<char>(keyEvent.keyCode);
    } else stream << "<keyCode=" << keyEvent.keyCode << ">";

    stream << " autorepeat=" << keyEvent.autoRepeat;

    stream << " }";

    return stream;
}

} // namespace EV