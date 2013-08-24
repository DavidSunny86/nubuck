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
        } frame;

        Metrics(void);
    };

    extern Metrics metrics;

} // namespace R