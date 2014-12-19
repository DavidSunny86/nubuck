#include <QToolBox>
#include <QFormLayout>

#include <Nubuck\math\math.h>
#include <renderer\renderer.h>
#include <UI\renderview\renderview.h>
#include <UI\mainwindow\mainwindow.h>
#include <UI\userinterface.h>
#include <UI\dirlight\dirlight.h>
#include "renderconfig.h"

namespace UI {

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

QWidget* RenderConfig::CreateBackgroundItem(RenderView* renderView) {
    ColorButton* btnBgColor = new ColorButton;
    btnBgColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    R::Color bgColor = renderView->GetBackgroundColor();
    btnBgColor->SetColor(bgColor.r, bgColor.g, bgColor.b);
    connect(btnBgColor, SIGNAL(SigColorChanged(float, float, float)), renderView, SLOT(OnSetBackgroundColor(float, float, float)));

    QCheckBox* cbGradient = new QCheckBox("Gradient");
    cbGradient->setChecked(renderView->IsBackgroundGradient());
    connect(cbGradient, SIGNAL(stateChanged(int)), renderView, SLOT(OnShowBackgroundGradient(int)));

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Color: "));
    hbox->addWidget(btnBgColor);
    hbox->addWidget(cbGradient);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addStretch();

    QWidget* backgroundItem = new QWidget;
    backgroundItem->setLayout(vbox);

    return backgroundItem;
}

QWidget* RenderConfig::CreateTransparencyItem() {
    QFormLayout* form = new QFormLayout;

    QComboBox* cbTransparencyMode = new QComboBox;
    cbTransparencyMode->addItem("Backfaces, Frontfaces");
    cbTransparencyMode->addItem("Sorted Triangles");
    cbTransparencyMode->addItem("Depth Peeling");
    connect(cbTransparencyMode, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTransparencyModeChanged(int)));

    QSpinBox* sbNumDepthPeels = new QSpinBox;
    sbNumDepthPeels->setMinimum(1);
    sbNumDepthPeels->setMaximum(10);
    sbNumDepthPeels->setValue(cvar_r_numDepthPeels);
    connect(sbNumDepthPeels, SIGNAL(valueChanged(int)), this, SLOT(OnNumDepthPeelsChanged(int)));

    form->addRow("Mode: ", cbTransparencyMode);
    form->addRow("Number of peels: ", sbNumDepthPeels);

    QWidget* transparencyItem = new QWidget;
    transparencyItem->setLayout(form);

    return transparencyItem;
}

QWidget* RenderConfig::CreateLightingItem() {
    DirLight* dirLight = new DirLight;
    return dirLight;
}

RenderConfig::RenderConfig(RenderView* renderView, QWidget* parent) : QDockWidget(parent) {
    setWindowTitle("Render Config");

    QToolBox* toolBox = new QToolBox;

    toolBox->addItem(CreateBackgroundItem(renderView), "Background");
    toolBox->addItem(CreateTransparencyItem(), "Transparency");
    toolBox->addItem(CreateLightingItem(), "Lighting");

    setWidget(toolBox);
}

} // namespace UI