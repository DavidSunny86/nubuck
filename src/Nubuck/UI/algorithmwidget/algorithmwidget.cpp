#include <algdriver\algdriver.h>
#include <world\events.h>
#include <world\world.h>
#include "algorithmwidget.h"

namespace UI {

    void AlgorithmWidget::OnStep(void) {
        ALG::gs_algorithm.Step();
    }

    void AlgorithmWidget::OnNext(void) {
        ALG::gs_algorithm.Next();
    }

    void AlgorithmWidget::OnRun(void) {
        ALG::gs_algorithm.Run();
    }

    void AlgorithmWidget::OnReset(void) {
        // TODO uncomment
        // ALG::gs_algorithm.Reset();
    }

    AlgorithmWidget::AlgorithmWidget(QWidget* parent) {
        _ui.setupUi(this);
    }

} // namespace UI