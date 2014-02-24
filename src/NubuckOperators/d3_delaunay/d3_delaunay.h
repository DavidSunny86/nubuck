#pragma once

#define NUBUCK_LIB

#include <QWidget>

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>

class QPushButton;

namespace OP {

class D3_DelaunayPanel : public QWidget {
    Q_OBJECT
private:
    unsigned        _nameIdx;
    QPushButton*    _button;
    QWidget*        _panel;
    void BuildPanelWidget();
private slots:
    void OnChangeButtonName();
public:
	D3_DelaunayPanel(QWidget* parent = NULL) : QWidget(parent) { }
};

class D3_Delaunay : public Operator {
private:
    Nubuck _nb;
public:
	D3_Delaunay() { }

    void Register(const Nubuck& nb, Invoker& invoker);
    void Invoke();
    void Finish();
};

} // namespace OP

extern "C" NUBUCK_API QWidget*      CreateOperatorPanel();
extern "C" NUBUCK_API OP::Operator* CreateOperator();