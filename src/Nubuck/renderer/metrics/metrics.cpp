#include "metrics.h"

namespace R {

    Metrics metrics;

    Metrics::Metrics(void) {
        resources.totalVertexBufferSize     = 0;
        resources.totalTextureBufferSize    = 0;
        frame.numDrawCalls                  = 0;
        frame.time                          = 0.0f;
        frame.edgeRendererSetTransformAccu  = 0.0f;
    }

    void Metrics::EndFrame() {
        frame.edgeRendererSetTransform = frame.edgeRendererSetTransformAccu;
        frame.edgeRendererSetTransformAccu = 0.0f;
    }

} // namespace R