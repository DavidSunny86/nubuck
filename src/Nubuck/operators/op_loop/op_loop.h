#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include <Nubuck\events\events.h>
#include <operators\operators.h>

#include <Nubuck\operators\operator.h>
#include <Nubuck\editors\vertex_editor.h>
#include <world\entities\ent_geometry\ent_geometry.h>
#include <world\entities\ent_transform_gizmo\ent_transform_gizmo.h>

namespace OP {

struct LoopPanel : QObject, OperatorPanel {
    Q_OBJECT
public slots:
    void OnButtonClicked();
public:
    LoopPanel() {
        QPushButton* button = new QPushButton("Start Loop");
        QVBoxLayout* layout = new QVBoxLayout;
        layout->addWidget(button);

        for(int i = 0; i < 3; ++i) {
            NB::Button otherButton = NB::CreateButton(i, "Hello, World!");
            NB::AddWidgetToBox(layout, NB::CastToWidget(otherButton));
        }

        layout->addStretch();
        GetWidget()->setLayout(layout);
        QObject::connect(button, SIGNAL(clicked()), this, SLOT(OnButtonClicked()));
    }
};

class Loop : public Operator {
private:
    W::ENT_Geometry*        _geom;
    NB::VertexEditor        _vertexEditor;

    void Event_OP_Loop_Start(const EV::Event& event);
    void Event_ButtonClicked(const EV::Event& event);
    void Event_Button0(const EV::Event& event);
    void Event_Button1(const EV::Event& event);
    void Event_Mouse(const EV::MouseEvent& event);
public:
    Loop();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }

    void OnMouse(const EV::MouseEvent& event) override;
};

} // namespace OP