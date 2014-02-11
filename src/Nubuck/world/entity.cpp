#include <common\common.h>
#include <world\world_events.h>
#include <world\world.h>
#include "entity.h"

namespace W {

M::Vector3 Entity::Transform(const M::Vector3& v) const {
    M::Vector3 r;
    r = M::Vector3(_transform.scale.x * v.x, _transform.scale.y * v.y, _transform.scale.z * v.z);
    r = M::Transform(_transform.rotation, r);
    r += _transform.position;
    return r;
}

M::Matrix4 Entity::GetTransformationMatrix() const {
    M::Matrix4 R = M::Mat4::FromRigidTransform(_transform.rotation, M::Vector3::Zero);
    M::Matrix4 T = M::Mat4::Translate(_transform.position);
    return T * R;
}

void Entity::Destroy() {
    EV::Params_DestroyEntity args;
    args.entId = GetID();
    W::world.Send(EV::def_DestroyEntity.Create(args));
}

} // namespace W