#pragma once

#define NUBUCK_LIB

#include <QWidget>

#include <Nubuck\nubuck.h>
#include <Nubuck\operators\operator.h>

class QPushButton;

namespace OP {

class D3_Delaunay : public QObject, public Operator {
    Q_OBJECT
private:
    Nubuck _nb;
    unsigned _nameIdx;
    QPushButton* _button;
    QWidget* _panel;
    void BuildPanelWidget();
private slots:
    void OnChangeButtonName();
public:
	D3_Delaunay() : _nameIdx(0), _panel(NULL) { }

    void Register(const Nubuck& nb, Invoker& invoker);
    void Invoke();
    void Finish();
};

} // namespace OP

extern "C" NUBUCK_API OP::Operator* CreateOperator();