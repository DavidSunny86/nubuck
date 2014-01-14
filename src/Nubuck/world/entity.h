#pragma once

#include <math\vector3.h>
#include <math\matrix4.h>

namespace W {

struct EntTransform {
    M::Vector3  position;
    M::Matrix4  rotation;
    M::Vector3  scale;
};

struct EntityType {
    enum Enum {
        ENT_POLYHEDRON  = 0,
        ENT_MESH,
        ENT_GEOMETRY
    };
};

class Entity {
private:
    EntityType::Enum    _type;
    unsigned            _entId;
    std::string         _name;
    std::string     	_fxName;
    EntTransform       	_transform;
public:
    virtual ~Entity() { }

    EntityType::Enum    GetType() const { return _type; }
    unsigned            GetID() const { return _entId; }
    const std::string&  GetName() const { return _name; }
    const std::string&  GetFxName() const { return _fxName; }
    const EntTransform& GetTransform() const { return _transform; }

    EntTransform&       GetTransform() { return _transform; }

    void                SetType(EntityType::Enum type) { _type = type; }
    void                SetID(unsigned id) { _entId = id; }
    void                SetName(const std::string& name) { _name = name; }
    void                SetFxName(const std::string& fxName) { _fxName = fxName; }
    void                SetTransform(const EntTransform& t) { _transform = t; }

    void                Destroy();
};

} // namespace W