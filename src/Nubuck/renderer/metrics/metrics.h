#pragma once

namespace R {

    struct Metrics {
        struct {
            unsigned totalVertexBufferSize;
            unsigned totalTextureBufferSize;
        } resources;

        Metrics(void);
    };

    extern Metrics metrics;

} // namespace R