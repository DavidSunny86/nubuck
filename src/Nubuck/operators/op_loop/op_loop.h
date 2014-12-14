#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include <Nubuck\events\events.h>
#include <operators\operators.h>

#include <Nubuck\operators\operator.h>
#include <world\entities\ent_geometry\ent_geometry.h>

namespace OP {

struct LoopPanel : OperatorPanel {
    Q_OBJECT
public slots:
    void OnButtonClicked();
public:
    LoopPanel(QWidget* parent = NULL) : OperatorPanel(parent) {
        QPushButton* button = new QPushButton("Start Loop");
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(button);
        layout->addStretch();
        setLayout(layout);
        connect(button, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    }
};

class Loop : public Operator {
private:
    W::ENT_Geometry* _geom;

    void Event_OP_Loop_Start(const EV::Event& event);
public:
    Loop();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP