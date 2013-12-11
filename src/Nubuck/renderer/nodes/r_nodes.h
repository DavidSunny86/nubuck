#pragma once

#include <vector>
#include <generic\uncopyable.h>
#include <generic\pointer.h>
#include <math\vector2.h>
#include <math\vector3.h>
#include <system\locks\spinlock.h>
#include <renderer\renderer.h>
#include <renderer\color\color.h>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\staticbuffer.h>

namespace R {

    class Nodes : private GEN::Uncopyable, public Renderable {
    public:
        struct Node {
            M::Vector3  position;
            R::Color    color;
        };
    private:
        struct BillboardHotVertex {
            M::Vector3  position;
            Color       color;
        };

        struct BillboardHot {
            BillboardHotVertex verts[4];
        };

        struct BillboardColdVertex {
            M::Vector2 texCoords; // used for clipping
        };

        struct BillboardCold {
            BillboardColdVertex verts[4];
        };

        SYS::SpinLock                   _stagedNodesMtx;
        std::vector<Node>               _stagedNodes;

        std::vector<Node>               _nodes;
        std::vector<Mesh::Index>        billboardIndices;
        std::vector<BillboardHot>    	billboardsHot;
        std::vector<BillboardCold>   	billboardsCold;
        GEN::Pointer<StaticBuffer>   	billboardHotVertexBuffer;
        GEN::Pointer<StaticBuffer>   	billboardColdVertexBuffer;
        GEN::Pointer<StaticBuffer>   	billboardIndexBuffer;

        void ReserveBillboards(void);
        void ReserveBillboardBuffers(void);
        void BuildBillboards(const M::Matrix4& worldMat);
        void BindHotBillboardAttributes(void);
        void BindColdBillboardAttributes(void);
        void UnbindAttributes(void);
        void DrawBillboards(const M::Matrix4& worldMat, const M::Matrix4& projectionMat);
    public:
        void Draw(const std::vector<Node>& nodes);

        void R_Prepare(const M::Matrix4& worldMat) override;
        void R_Draw(const M::Matrix4& worldMat, const M::Matrix4& projectionMat) override;
    };

    extern Nodes g_nodes;

} // namespace R