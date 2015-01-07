#pragma once

#include <Nubuck\operators\operator.h>

class QComboBox;

namespace OP {
// namespace GEN {

class PlatonicSolidsPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private:
    QComboBox* _names;
private slots:
    void OnNameChanged(int idx);
public:
    PlatonicSolidsPanel();

    void Invoke() override;
};

class PlatonicSolids : public Operator {
private:
    NB::Mesh _mesh;

    void CreateMesh(int type);

    void Event_CreatePlatonicSolid(const EV::Arg<int>& event);
public:
    PlatonicSolids();

    void Register(Invoker& invoker) override;
    bool Invoke() override;
    void Finish() override { }
};

// } // namespace GEN
} // namespace OP