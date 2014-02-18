#include <common\common.h>
#include <system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
#include "entity.h"

namespace W {

Entity::Entity() : _tags(0) { }

void Entity::Select() {
    SYS::ScopedLock lock(_mtx);
	_tags |= Tags::IS_SELECTED;
}

void Entity::Deselect() {
    SYS::ScopedLock lock(_mtx);
	_tags &= ~Tags::IS_SELECTED;
}

bool Entity::IsSelected() const {
	return _tags & Tags::IS_SELECTED;
}

bool Entity::IsDead() const {
	return _tags & Tags::IS_DEAD;
}

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
    // no lock necessary!
    _tags |= Tags::IS_DEAD;
}

} // namespace W