#pragma once

#include <QWidget>
#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include "ui_op_loadobj.h"

namespace W {

class ENT_Geometry;

}; // namespace W

namespace OP {

class LoadOBJPanel : public OperatorPanel {
    Q_OBJECT
private:
	Ui::LoadOBJ _ui;
private slots:
    void OnChooseFilename();
    void OnLoadScene();
public:
    LoadOBJPanel(QWidget* parent = NULL);

    void Invoke() override;
};

class LoadOBJ : public Operator {
private:
	W::ENT_Geometry*    _geom;

    void Event_Load(const EV::Arg<QString*>& event);
    void Event_LoadScene(const EV::Event& event);
public:
	LoadOBJ();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

} // namespace OP