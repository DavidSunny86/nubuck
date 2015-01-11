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

class LoadOBJPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private:
	Ui::LoadOBJ _ui;
private slots:
    void OnChooseFilename();
    void OnLoadScene();
public:
    LoadOBJPanel();

    void Invoke() override;
};

class LoadOBJ : public Operator {
private:
	NB::Mesh _mesh;

    void Event_Load(const EV::Arg<QString*>& event);
    void Event_LoadScene(const EV::Event& event);
public:
	LoadOBJ();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
    bool IsDone() const override { return true; }
};

} // namespace OP