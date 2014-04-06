#pragma once

namespace R {

    struct Metrics {
        struct {
            unsigned totalVertexBufferSize;
            unsigned totalTextureBufferSize;
        } resources;

        struct {
            unsigned    numDrawCalls;
            float       time;
            float       edgeRendererSetTransformAccu;
            float       edgeRendererSetTransform;
        } frame;

        Metrics(void);

        void EndFrame();
    };

    extern Metrics metrics;

} // namespace R