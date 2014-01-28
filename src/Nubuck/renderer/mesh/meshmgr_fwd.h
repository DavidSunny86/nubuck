#pragma once

namespace R {

class Mesh;
class TFMesh;

namespace MeshMgr_Impl {

template<typename TMESH> struct MeshLink;

} // namespace MeshMgr_Impl

typedef MeshMgr_Impl::MeshLink<Mesh>*   meshPtr_t;
typedef MeshMgr_Impl::MeshLink<TFMesh>* tfmeshPtr_t;

} // namespace R