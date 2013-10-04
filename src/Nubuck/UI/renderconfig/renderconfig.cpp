#include <math\math.h>
#include <renderer\renderer.h>
#include "renderconfig.h"

namespace UI {

void RenderConfig::OnNodeSizeChanged(double value) { 
    float diff = (float)_ui.sbNodeSize->value() - cvar_r_nodeSize;
    if(0.0f != diff) {
        cvar_r_nodeSize = cvar_r_nodeSize + diff;
        if(_ui.cbLink->isChecked()) {
            cvar_r_edgeRadius = M::Max((float)_ui.sbEdgeRadius->minimum(), cvar_r_edgeRadius + diff);
            _ui.sbEdgeRadius->setValue(cvar_r_edgeRadius);
        }
    }
}

void RenderConfig::OnEdgeRadiusChanged(double value) {
    float diff = (float)_ui.sbEdgeRadius->value() - cvar_r_edgeRadius;
    if(0.0f != diff) {
        cvar_r_edgeRadius = cvar_r_edgeRadius + diff;
        if(_ui.cbLink->isChecked()) {
            cvar_r_nodeSize = M::Max((float)_ui.sbNodeSize->minimum(), cvar_r_nodeSize + diff);
            _ui.sbNodeSize->setValue(cvar_r_nodeSize);
        }
    }
}

RenderConfig::RenderConfig(QWidget* parent) : QDockWidget(parent) {
    _ui.setupUi(this);

    _ui.sbNodeSize->setValue(cvar_r_nodeSize);
    _ui.sbEdgeRadius->setValue(cvar_r_edgeRadius);
}

} // namespace UI