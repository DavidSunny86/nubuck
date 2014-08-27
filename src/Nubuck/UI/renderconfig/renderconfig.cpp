#include <Nubuck\math\math.h>
#include <renderer\renderer.h>
#include <UI\renderview\renderview.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\userinterface.h>
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

void RenderConfig::OnTransparencyModeChanged(int index) {
    // combobox order must conform enum
    assert(0 <= index && index < R::TransparencyMode::NUM_MODES);
    cvar_r_transparencyMode = index;
    std::cout << "RenderConfig::OnTransparencyModeChanged: index = " << index << std::endl;
}

void RenderConfig::OnNumDepthPeelsChanged(int value) {
    if(cvar_r_numDepthPeels != value) {
        cvar_r_numDepthPeels = value;
    }
}

RenderConfig::RenderConfig(RenderView* renderView, QWidget* parent) : QDockWidget(parent) {
    _ui.setupUi(this);

    _ui.sbNodeSize->setValue(cvar_r_nodeSize);
    _ui.sbEdgeRadius->setValue(cvar_r_edgeRadius);
    _ui.sbNumDepthPeels->setValue(cvar_r_numDepthPeels);

    R::Color bgColor = renderView->GetBackgroundColor();
    _ui.btnBgColor->SetColor(bgColor.r, bgColor.g, bgColor.b);
    _ui.cbGradient->setChecked(renderView->IsBackgroundGradient());
    connect(_ui.btnBgColor, SIGNAL(SigColorChanged(float, float, float)), renderView, SLOT(OnSetBackgroundColor(float, float, float)));
    connect(_ui.cbGradient, SIGNAL(stateChanged(int)), renderView, SLOT(OnShowBackgroundGradient(int)));
}

} // namespace UI