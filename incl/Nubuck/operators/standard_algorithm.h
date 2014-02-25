#pragma once

#include <string>

#include <Nubuck\operators\operator.h>

namespace OP {
namespace ALG {

class NUBUCK_API StandardAlgorithmPanel : public OperatorPanel {
    Q_OBJECT
private slots:
    void OnStep();
public:
    StandardAlgorithmPanel(QWidget* parent = NULL);
};

struct NUBUCK_API Phase {
    virtual void Enter() { }
};

class NUBUCK_API StandardAlgorithm : public Operator {
private:
    Nubuck  _nb;
    Phase*  _phase;

    void SetPhase(Phase* phase);
protected:
    virtual const char* GetName() const = 0;
    virtual Phase*      Init(const Nubuck& nb) = 0;
public:
    StandardAlgorithm();

    void Register(const Nubuck& nb, Invoker& invoker) override;

    void Invoke() override;
    void Finish() override { }
};

} // namespace ALG
} // namespace OP