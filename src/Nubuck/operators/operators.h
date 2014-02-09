#pragma once

#include <assert.h>

#include <QObject>

#include <vector>
#include <Nubuck\operators\operator.h>
#include <UI\operatorpanel\operatorpanel.h>
#include <renderer\renderer.h>

namespace OP {

class Operators : public QObject {
    Q_OBJECT
private:
    struct OperatorDesc {
        unsigned    id;
        Operator*   op;
        Invoker*    invoker;
        HMODULE     module;
    };

    std::vector<OperatorDesc>   _ops;       // all registered operators
    std::vector<Operator*>      _activeOps; // active operators

    void UnloadModules();
public slots:
    void OnInvokeOperator(unsigned id);
public:
    Operators() { }
    ~Operators();

    unsigned Register(Operator* op, HMODULE module = NULL);

    void SetInitOp(unsigned id);

    Operator* ActiveOperator();

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs);

    void SelectGeometry();

    void OnCameraChanged();
    bool OnMouseDown(const M::Vector2& mouseCoords, bool shiftKey);
    bool OnMouseMove(const M::Vector2& mouseCoords);
    bool OnMouseUp(const M::Vector2& mouseCoords);
};

extern Operators g_operators;

} // namespace OP