#pragma once

#include <qdockwidget>
#include "ui_renderconfig.h"

namespace UI {

class ColorButton;
class RenderView;

class RenderConfig : public QDockWidget {
    Q_OBJECT
private:
    Ui::RenderConfig _ui;
public slots:
    void OnNodeSizeChanged(double value);
    void OnEdgeRadiusChanged(double value);
    void OnTransparencyModeChanged(int index);
    void OnNumDepthPeelsChanged(int value);
public:
    RenderConfig(RenderView* renderView, QWidget* parent = NULL);
};

} // namespace UI