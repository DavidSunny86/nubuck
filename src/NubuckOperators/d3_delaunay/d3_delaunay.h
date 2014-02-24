#pragma once

#define NUBUCK_LIB

#include <QWidget>

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>

BEGIN_EVENT_DEF(D3_Delaunay_Action)
END_EVENT_DEF

class QPushButton;

namespace OP {

class D3_DelaunayPanel : public OperatorPanel {
    Q_OBJECT
private:
    QString         _names[2];
    unsigned        _nameIdx;
    QPushButton*    _button;
    QWidget*        _panel;
    void BuildPanelWidget();
private slots:
    void OnChangeButtonName();
public:
	D3_DelaunayPanel(QWidget* parent = NULL);
};

class D3_Delaunay : public Operator {
private:
    Nubuck _nb;

    void Event_Action(const EV::Event& args);
public:
	D3_Delaunay();

    void Register(const Nubuck& nb, Invoker& invoker);
    void Invoke();
    void Finish();
};

} // namespace OP

extern "C" NUBUCK_API QWidget*      CreateOperatorPanel();
extern "C" NUBUCK_API OP::Operator* CreateOperator();