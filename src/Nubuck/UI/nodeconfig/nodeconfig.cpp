#include <renderer\renderer.h>
#include "nodeconfig.h"

namespace UI {

	void NodeConfig::OnTypeChanged(int idx) {
		// keep combobox consistent with NodeRenderType in renderer.h
		cvar_r_nodeType = idx;
	}

    void NodeConfig::OnSizeChanged(double val) { cvar_r_nodeSize = (float)val; }
    void NodeConfig::OnSubdivisionsChanged(int val) { cvar_r_nodeSubdiv = val; }
    void NodeConfig::OnSmoothChanged(int val) { cvar_r_nodeSmooth = Qt::Checked == val; }

    NodeConfig::NodeConfig(QWidget* parent) : QDialog(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint);

        _ui.setupUi(this);

        _ui.sbSize->setValue(cvar_r_nodeSize);
        _ui.sbSubdiv->setValue(cvar_r_nodeSubdiv);

        _ui.cbSmooth->setCheckState(Qt::Checked);
    }

} // namespace UI