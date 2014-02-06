#pragma once

#include <QWidget>
#include <Nubuck\nubuck.h>
#include <operators\operator.h>
#include <operators\operators.h>
#include "ui_op_loadobj.h"

namespace W {

class ENT_Geometry;

}; // namespace W

namespace OP {

class LoadOBJ : public QObject, public Operator {
    Q_OBJECT
private:
    Nubuck              _nb;
	W::ENT_Geometry*    _geom;

	Ui::LoadOBJ _ui;
    QWidget*    _panel;

    void BuildPanel();
public slots:
    void OnChooseFilename();
public:
	LoadOBJ() : _geom(NULL), _panel(NULL) { }

    void Register(const Nubuck& nb, Invoker& invoker) override;
    void Invoke() override;
    void Finish() override { }
};

} // namespace OP