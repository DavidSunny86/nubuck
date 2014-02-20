#pragma once

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix3.h>

namespace W {

struct EntTransform {
    M::Vector3  position;
    M::Matrix3  rotation;
    M::Vector3  scale;
};

struct EntityType {
    enum Enum {
        ENT_GEOMETRY = 0
    };
};

class Entity {
private:
    struct Tags {
        enum Enum {
            IS_DEAD     = (1 << 0),
            IS_SELECTED = (1 << 1)
		};
	};

	mutable SYS::SpinLock _mtx;

    int _tags;

    EntityType::Enum    _type;
    unsigned            _entId;
    std::string         _name;
    std::string     	_fxName;
    EntTransform       	_transform;
public:
    Entity();
    virtual ~Entity() { }

    void Select();
    void Deselect();
    bool IsSelected() const;

    bool IsDead() const;

    EntityType::Enum    GetType() const { return _type; }
    unsigned            GetID() const { return _entId; }
    const std::string&  GetName() const { return _name; }
    const std::string&  GetFxName() const { return _fxName; }
    const EntTransform& GetTransform() const { return _transform; }

    EntTransform&       GetTransform() { return _transform; }
    M::Vector3          Transform(const M::Vector3& v) const;
    M::Matrix4          GetTransformationMatrix() const;

    void                Translate(const M::Vector3& v);

    void                SetType(EntityType::Enum type) { _type = type; }
    void                SetID(unsigned id) { _entId = id; }
    void                SetName(const std::string& name) { _name = name; }
    void                SetFxName(const std::string& fxName) { _fxName = fxName; }
    void                SetTransform(const EntTransform& t) { _transform = t; }

    void                Destroy();
	virtual void        OnDestroy() { }
};

} // namespace W