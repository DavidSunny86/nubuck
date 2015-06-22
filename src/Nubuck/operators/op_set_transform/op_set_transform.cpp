#include <Nubuck\math_conv.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "op_set_transform.h"

namespace OP {

void SetTransform::Register(Invoker& invoker) {
}

void SetTransform::Event_EntUsrSetPosition(const SetEntityVectorEvent& event) {
    W::Entity* ent = W::world.GetEntityByID(event.m_entityID);
    M::Vector3 position = ToVector(NB::Point3(event.m_vector[0], event.m_vector[1], event.m_vector[2]));
    COM_assert(ent);
    ent->SetPosition(position);

    // TODO: why does the application crash if we dont accept this event at this point?
    // worst thing that should happen is that the world tries to invoke this operator again.
    // oh well...
    event.Accept();
}

SetTransform::SetTransform() {
    AddEventHandler(ev_ent_usr_setPosition, this, &SetTransform::Event_EntUsrSetPosition);
}

bool SetTransform::Invoke() {
    NB::SetOperatorName("Set Transformation");
    return true;
}

} // namespace OP