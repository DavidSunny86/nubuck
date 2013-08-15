#pragma once

namespace R {

    struct Metrics {
        struct {
            unsigned totalVertexBufferSize;
            unsigned totalTextureBufferSize;
        } resources;

        struct {
            unsigned numDrawCalls;
        } frame;

        Metrics(void);
    };

    extern Metrics metrics;

} // namespace R