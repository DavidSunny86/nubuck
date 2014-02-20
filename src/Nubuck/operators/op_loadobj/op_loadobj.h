#pragma once

#include <QWidget>
#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>
#include <operators\operators.h>
#include "ui_op_loadobj.h"

BEGIN_EVENT_DEF(OP_LoadOBJ_Load)
    QString* filename;
END_EVENT_DEF

BEGIN_EVENT_DEF(OP_LoadOBJ_LoadScene)
END_EVENT_DEF

namespace W {

class ENT_Geometry;

}; // namespace W

namespace OP {

class LoadOBJPanel : public QWidget {
    Q_OBJECT
private:
	Ui::LoadOBJ _ui;
private slots:
    void OnChooseFilename();
    void OnLoadScene();
public:
    LoadOBJPanel(QWidget* parent = NULL);
};

class LoadOBJ : public Operator {
private:
    Nubuck              _nb;
	W::ENT_Geometry*    _geom;

    void Event_Load(const EV::Event& event);
    void Event_LoadScene(const EV::Event& event);
public:
	LoadOBJ();

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP