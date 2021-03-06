#pragma once

#include <string>

#include <QObject>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\operators\operator.h>
#include <Nubuck\events\events.h>
#include <Nubuck\operators\standard_algorithm_fwd.h>

namespace OP {
namespace ALG {

class NUBUCK_API StandardAlgorithmPanel : public QObject, public OperatorPanel {
    Q_OBJECT
private slots:
    void OnStep();
    void OnNext();
    void OnRun();
public:
    StandardAlgorithmPanel();
};

class NUBUCK_API Phase {
public:
    struct RunMode { enum Enum { STEP = 0, NEXT, RUN }; };

    struct RunConf {
        RunMode::Enum mode;
    };
private:
    const RunConf* _runConf;
public:
	virtual ~Phase() { }

    void            SetRunConf(const RunConf* runConf) { _runConf = runConf; }
    const RunConf&  GetRunConf() const { return *_runConf; }

    virtual void Enter() { }
    virtual void Leave() { }

    struct StepRet { enum Enum { DONE = 0, CONTINUE }; };

    virtual StepRet::Enum       Step();
	virtual GEN::Pointer<Phase> NextPhase();

    virtual bool                IsWall() const;
};

class NUBUCK_API StandardAlgorithm : public Operator {
private:
    GEN::Pointer<Phase> _phase;
    Phase::RunConf      _runConf;

    void SetPhase(const GEN::Pointer<Phase>& phase);

    void Event_Step(const EV::Event& event);
    void Event_Next(const EV::Event& event);
    void Event_Run(const EV::Event& event);
protected:
    virtual const char* GetName() const = 0;
    virtual Phase*      Init() = 0; // return false to decline invocation
public:
    StandardAlgorithm();

    void Register(Invoker& invoker) override;

    bool Invoke() override;
    void Finish() override;

	void OnMouse(const EV::MouseEvent& mouseEvent) override;
    void OnKey(const EV::KeyEvent& keyEvent) override;
};

} // namespace ALG
} // namespace OP