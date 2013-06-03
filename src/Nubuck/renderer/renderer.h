#pragma once

#include <vector>

#include <generic\pointer.h>
#include <system\locks\spinlock.h>
#include <system\timer\timer.h>
#include <math\matrix4.h>
#include "material\material.h"
#include "mesh\meshmgr.h"
#include "skin\skinmgr.h"
#include "light\light.h"

namespace R {
	
    class   Effect;
    class   Mesh;

    struct RenderJob {
        std::string             fx;
        MeshMgr::vertexHandle_t vertices;
        MeshMgr::indexHandle_t  indices;
        GLenum                  primType;
        Material	            material;
        SkinMgr::handle_t       skin;
        M::Matrix4              transform;

        // handled by renderer
        RenderJob* next;
    };

	class Renderer {
    private:
        std::vector<RenderJob>  _renderList;
        SYS::SpinLock           _renderListLock;

        std::vector<RenderJob>  _renderJobs;
        std::vector<Light>      _lights;

        SYS::Timer  _timer;
        float       _time;

        void DrawFrame(const M::Matrix4& worldMat, const M::Matrix4& projectionMat);
	public:
        Renderer(void);

        void Init(void); // requires gl context

        void Resize(int width, int height);

        void Add(const Light& light);

        void SetRenderList(const std::vector<RenderJob>& renderList);
        void Render(const M::Matrix4& worldMat, const M::Matrix4& projectionMat);
	};

} // namespace R
