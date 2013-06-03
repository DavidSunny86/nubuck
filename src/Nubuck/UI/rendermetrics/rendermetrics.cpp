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

    void RenderMetrics::Update(void) {
        _ui.lblTotalVertexBufferSizeValue->setText(FormatBytes(R::metrics.resources.totalVertexBufferSize));
        _ui.lblTotalTextureBufferSizeValue->setText(FormatBytes(R::metrics.resources.totalTextureBufferSize));
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