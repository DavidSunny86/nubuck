#include <events\event_defs.h>
#include <world\world.h>
#include <world\entities\ent_polyhedron\ent_polyhedron.h>
#include "proxy_mesh.h"

namespace Proxy {

    void Mesh::SetPosition(float x, float y, float z) {
        EV::Params_SetPosition args;
        args.entId = _entId;
        args.pos = M::Vector3(x, y, z);
        W::world.Send(EV::def_SetPosition.Create(args));
    }

    void Mesh::SetScale(float sx, float sy, float sz) {
        EV::Params_SetScale args;
        args.entId = _entId;
        args.sx = sx;
        args.sy = sy;
        args.sz = sz;
        W::world.Send(EV::def_SetScale.Create(args));
    }

    // rotates the coordinate space such that the new z axis coincides with the vector d.
    // example. AlignZ(d) * d = Length(d) * (0, 0, 1)
    static M::Matrix4 _AlignZ(const M::Vector3& d) {
        const float len_yz_sq = d.y * d.y + d.z * d.z;
        const float len = d.Length();
        assert(0.0f < len);
        if(0.0f == len_yz_sq) { // case d, x collinear => rot_x = 0
            return M::Matrix4(
                0.0f, 0.0f, -d.x / len, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                d.x / len, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            );
        } else {
            const float len_yz = sqrt(len_yz_sq);
            return M::Matrix4(
                len_yz * len_yz, -d.x * d.y, -d.x * d.z, 0.0f,
                0.0f, len * d.z, -len * d.y, 0.0f,
                len_yz * d.x, len_yz * d.y, len_yz * d.z, 0.0f,
                0.0f, 0.0f, 0.0f, len_yz * len
            ) / (len_yz * len);
        }
    }

    void Mesh::AlignZ(float x, float y, float z) {
        M::Vector3 d = M::Normalize(M::Vector3(x, y, z));
        EV::Params_SetRotation args;
        args.entId = _entId;
        args.mat = M::RotationOf(_AlignZ(d));
        W::world.Send(EV::def_SetRotation.Create(args));
    }

    void Mesh::SetOrient(float x0, float y0 ,float z0, float x1, float y1, float z1) {
        M::Vector3 x = M::Normalize(M::Vector3(x0, y0, z0));
        M::Vector3 z = M::Normalize(M::Vector3(x1, y1, z1));
        M::Vector3 y = M::Normalize(M::Cross(z, x));
        M::Matrix3 m = M::Mat3::FromColumns(x, y, z);
        EV::Params_SetRotation args;
        args.entId = _entId;
        args.mat = m;
        W::world.Send(EV::def_SetRotation.Create(args));
    }

    void Mesh::SetEffect(const char* fxName) {
        EV::Params_SetEffect args;
        args.entId = _entId;
        args.fxName = fxName;
        W::world.Send(EV::def_SetEffect.Create(args));
    }

    void Mesh::SetVisible(bool isVisible) {
        EV::Params_SetVisible args;
        args.entId = _entId;
        args.isVisible = isVisible;
        W::world.Send(EV::def_SetVisible.Create(args));
    }

} // namespace Proxy