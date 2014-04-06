#include <QLabel>

#include <renderer\metrics\metrics.h>
#include "rendermetrics.h"

namespace UI { 

    static QString FormatBytes(unsigned size) {
        enum Multiples {
            BYTES       = 0,
            KILO_BYTES,
            MEGA_BYTES,
            GIGA_BYTES,

            NUM_MULTIPLES
        };
        const char* units[] = {
            "B", "KB", "MB", "GB"
        };

        int mult = 0;
        while(1000 <= size && mult < NUM_MULTIPLES) {
            size /= 1000;
            mult++;
        }

        return QString("%1 %2").arg(size).arg(units[mult]);
    }

    static QString ToQString(float f) {
        static const int    float_width = 0;
        static const char   float_fmt = 'f';
        static const int    float_prec = 8;
        return QString("%1").arg((double)f, float_width, float_fmt, float_prec);
    }

    void RenderMetrics::Update(void) {
        _ui.lblTotalVertexBufferSizeValue->setText(FormatBytes(R::metrics.resources.totalVertexBufferSize));
        _ui.lblTotalTextureBufferSizeValue->setText(FormatBytes(R::metrics.resources.totalTextureBufferSize));
        _ui.lblNumDrawCallsValue->setText(QString("%1").arg(R::metrics.frame.numDrawCalls));
        _ui.lblFrameTimeValue->setText(ToQString(R::metrics.frame.time));
        _ui.lblEdgeRendererSetTransformValue->setText(ToQString(R::metrics.frame.edgeRendererSetTransform));
    }

    RenderMetrics::RenderMetrics(QWidget* parent) : QWidget(parent) {
        _ui.setupUi(this);

        const int timeout = 5000.0f; // in msec
        connect(&_timer, SIGNAL(timeout()), this, SLOT(Update()));
        _timer.start(timeout);
    }

    RenderMetrics* RenderMetrics::Instance(void) {
        static RenderMetrics* instance = NULL;
        if(!instance) instance = new RenderMetrics();
        return instance;
    }

} // namespace UI