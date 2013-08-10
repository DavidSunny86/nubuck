#include <algdriver\algdriver.h>
#include <world\events.h>
#include <world\world.h>
#include "algorithmwidget.h"

namespace UI {

    void AlgorithmWidget::OnStep(void) {
        ALG::Driver::Instance().Step();
    }

    void AlgorithmWidget::OnNext(void) {
        ALG::Driver::Instance().Next();
    }

    void AlgorithmWidget::OnRun(void) {
        ALG::Driver::Instance().Run();
    }

    void AlgorithmWidget::OnReset(void) {
        ALG::Driver::Instance().Reset();
    }

    AlgorithmWidget::AlgorithmWidget(QWidget* parent) {
        _ui.setupUi(this);
    }

} // namespace UI