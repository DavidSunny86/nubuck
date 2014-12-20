#pragma once

#include <qdockwidget>

namespace UI {

class ColorButton;
class RenderView;

class RenderConfig : public QDockWidget {
    Q_OBJECT
private:
    QWidget* CreateBackgroundItem(RenderView* renderView);
    QWidget* CreateTransparencyItem();
    QWidget* CreateLightingItem();
public slots:
    void OnTransparencyModeChanged(int index);
    void OnNumDepthPeelsChanged(int value);
public:
    RenderConfig(RenderView* renderView, QWidget* parent = NULL);
};

} // namespace UI