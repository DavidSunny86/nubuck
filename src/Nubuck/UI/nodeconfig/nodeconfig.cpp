#include <world\entities\ent_node\ent_node.h>
#include "nodeconfig.h"

namespace UI {

    void NodeConfig::OnSizeChanged(double val) { cvar_nodeSize = (float)val; }
    void NodeConfig::OnSubdivisionsChanged(int val) { cvar_nodeSubdiv = val; }

    NodeConfig::NodeConfig(QWidget* parent) : QDialog(parent) {
        setWindowFlags(Qt::WindowStaysOnTopHint);

        _ui.setupUi(this);

        _ui.sbSize->setValue(cvar_nodeSize);
        _ui.sbSubdiv->setValue(cvar_nodeSubdiv);
    }

} // namespace UI