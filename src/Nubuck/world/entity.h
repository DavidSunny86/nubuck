#pragma once

#include <string>

#include <Nubuck\math\vector3.h>
#include <Nubuck\math\matrix4.h>
#include <Nubuck\math\quaternion.h>
#include <Nubuck\system\locks\spinlock.h>
#include <UI\outliner\outliner_fwd.h>

namespace W {

struct EntityType {
    enum Enum {
        ENT_GEOMETRY = 0,
        ENT_TEXT
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

    M::Vector3          _position;
    M::Quaternion       _orientation;
    M::Vector3          _scale;
public:
    Entity();
    virtual ~Entity() { }

    void Select();
    void Deselect();
    bool IsSelected() const;

    bool IsDead() const;

    EntityType::Enum    GetType() const;
    unsigned            GetID() const;
    const std::string&  GetName() const;
    const std::string&  GetFxName() const;

    void                SetType(EntityType::Enum type);
    void                SetID(unsigned id);
    virtual void        SetName(const std::string& name);
    void                SetFxName(const std::string& fxName);

    M::Vector3          GetPosition() const;
    M::Quaternion       GetOrientation() const;
    M::Vector3          GetScale() const;

    M::Matrix4          GetObjectToWorldMatrix() const;

    void                SetPosition(const M::Vector3& position);
    void                SetOrientation(const M::Quaternion& orientation);
    void                SetScale(const M::Vector3& scale);

    void                Destroy();
	virtual void        OnDestroy() { }

    virtual UI::OutlinerView* CreateOutlinerView() = 0;
};

} // namespace W