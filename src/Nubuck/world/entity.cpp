#include <Nubuck\common\common.h>
#include <Nubuck\system\locks\scoped_lock.h>
#include <world\world_events.h>
#include <world\world.h>
#include "entity.h"

namespace W {

void Entity::SetBoundingBox(const M::Box& bbox) {
    _bbox = bbox;
}

Entity::Entity()
    : _tags(0)
    , _position(M::Vector3::Zero)
    , _orientation(M::Quat::Identity())
    , _scale(1.0f, 1.0f, 1.0f)
{ }

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

EntityType::Enum Entity::GetType() const {
    return _type;
}

unsigned Entity::GetID() const {
    return _entId;
}

const std::string& Entity::GetName() const {
    SYS::ScopedLock lock(_mtx);
    return _name;
}

const std::string& Entity::GetFxName() const {
    SYS::ScopedLock lock(_mtx);
    return _fxName;
}

void Entity::SetType(EntityType::Enum type) {
    _type = type;
}

void Entity::SetID(unsigned id) {
    _entId = id;
}

void Entity::SetName(const std::string& name) {
    SYS::ScopedLock lock(_mtx);
    _name = name;
}

void Entity::SetFxName(const std::string& fxName) {
    SYS::ScopedLock lock(_mtx);
    _fxName = fxName;
}

M::Vector3 Entity::GetPosition() const {
    SYS::ScopedLock lock(_mtx);
    return _position;
}

M::Quaternion Entity::GetOrientation() const {
    SYS::ScopedLock lock(_mtx);
    return _orientation;
}

M::Vector3 Entity::GetScale() const {
    SYS::ScopedLock lock(_mtx);
    return _scale;
}

const M::Box& Entity::GetBoundingBox() const {
    return _bbox;
}

M::Matrix4 Entity::GetObjectToWorldMatrix() const {
    SYS::ScopedLock lock(_mtx);
    return
        M::Mat4::Translate(_position) *
        M::Mat4::RotateQuaternion(_orientation) *
        M::Mat4::Scale(_scale.x, _scale.y, _scale.z);
}

M::Vector3 Entity::GetLocalCenter() const {
	SYS::ScopedLock lock(_mtx);
    return _bbox.min + 0.5f * (_bbox.max - _bbox.min);
}

M::Vector3 Entity::GetGlobalCenter() const {
	SYS::ScopedLock lock(_mtx);
    M::Vector3 localCenter = GetLocalCenter();
    return M::Transform(GetObjectToWorldMatrix(), localCenter);
}

void Entity::SetPosition(const M::Vector3& position) {
    SYS::ScopedLock lock(_mtx);
    _position = position;
}

void Entity::SetOrientation(const M::Quaternion& orientation) {
    SYS::ScopedLock lock(_mtx);
    _orientation = orientation;
}

void Entity::SetScale(const M::Vector3& scale) {
    SYS::ScopedLock lock(_mtx);
    _scale = scale;
}

void Entity::Destroy() {
    // no lock necessary!
    _tags |= Tags::IS_DEAD;
}

} // namespace W