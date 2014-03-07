#pragma once

#include <string>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\events\events.h>
#include <Nubuck\operators\standard_algorithm_fwd.h>

BEGIN_EVENT_DEF(ALG_Step) END_EVENT_DEF


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
	virtual ~Phase() { }

    virtual void Enter() { }

    struct StepRet { enum Enum { DONE = 0, CONTINUE }; };

    virtual StepRet::Enum       Step();
	virtual GEN::Pointer<Phase> NextPhase();
};

class NUBUCK_API StandardAlgorithm : public Operator {
private:
    Nubuck              _nb;
    GEN::Pointer<Phase> _phase;

    void SetPhase(const GEN::Pointer<Phase>& phase);

    void Event_Step(const EV::Event& event);
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