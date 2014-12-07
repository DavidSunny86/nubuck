#pragma once

#include <Nubuck\operators\operator.h>

BEGIN_EVENT_DEF(CreatePlatonicSolid)
    int type;
END_EVENT_DEF

class QComboBox;

namespace OP {
// namespace GEN {

class PlatonicSolidsPanel : public OperatorPanel {
    Q_OBJECT
private:
    QComboBox* _names;
private slots:
    void OnNameChanged(int idx);
public:
    PlatonicSolidsPanel(QWidget* parent = NULL);

    void Invoke() override;
};

class PlatonicSolids : public Operator {
private:
    nb::geometry _geom;

    void CreateMesh(int type);

    void Event_CreatePlatonicSolid(const EV::Event& event);
public:
    PlatonicSolids();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

// } // namespace GEN
} // namespace OP