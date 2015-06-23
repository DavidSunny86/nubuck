#include <Nubuck\math_conv.h>
#include <world\world.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include "entity_vector.h"
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

    EntityVector* const * vargs = reinterpret_cast<EntityVector* const *>(GetArgumentData());
    COM_assert(vargs);

    const EntityVector* args = vargs[0];
    COM_assert(args);

    COM_assert(EntityVector::MAGIC_PATTERN == args->m_magic);

    W::Entity* ent = W::world.GetEntityByID(args->m_entityID);
    COM_assert(ent);

    if(EntityVector::VectorType_Position == args->m_type) {
        NB::Point3 ratPos = NB::Point3(args->m_vector[0], args->m_vector[1], args->m_vector[2]);
        ent->SetPosition(ToVector(ratPos));
    } else {
        COM_assert(0 && "not yet implemented");
    }

    return true;
}

} // namespace OP