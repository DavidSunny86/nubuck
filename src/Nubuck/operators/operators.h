#pragma once

#include <assert.h>

#include <QObject>

#include <vector>
#include <UI\operatorpanel\operatorpanel.h>
#include <renderer\renderer.h>
#include "operator.h"

namespace OP {

class Invoker : public QObject {
    Q_OBJECT
private:
    unsigned _id;
signals:
    void SigInvokeOperator(unsigned id);
public slots:
    void OnInvoke() { emit SigInvokeOperator(_id); }
public:
    explicit Invoker(unsigned id) : _id(id) { }
};

class Operators : public QObject {
    Q_OBJECT
private:
    struct OperatorDesc {
        unsigned    id;
        Operator*   op;
        Invoker*    invoker;
    };

    std::vector<OperatorDesc>   _ops;       // all registered operators
    std::vector<Operator*>      _activeOps; // active operators
public slots:
    void OnInvokeOperator(unsigned id) {
        printf("invoking operator with id = %d\n", id);
        Operator* op = NULL;
        op = ActiveOperator();
        if(op) {
            op->Finish();
            if(1 < _activeOps.size()) _activeOps.pop_back();
        }
        UI::OperatorPanel::Instance()->Clear();
        op = _ops[id].op;
        _activeOps.push_back(op);
        op->Invoke();
    }
public:
    Operators() { }

    unsigned Register(Operator* op);

    void SetInitOp(unsigned id) {
        assert(_activeOps.empty());
        OnInvokeOperator(id);
    }

    Operator* ActiveOperator() {
        if(_activeOps.empty()) return NULL;
        return _activeOps.back();
    }

    void GetMeshJobs(std::vector<R::MeshJob>& meshJobs) {
        for(unsigned i = 0; i < _activeOps.size(); ++i)
            _activeOps[i]->GetMeshJobs(meshJobs);
    }

    void SelectGeometry() {
        for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
            _activeOps.rend() != it; ++it)
        {
            (*it)->OnGeometrySelected();
        }
    }

    void OnCameraChanged() {
        for(std::vector<Operator*>::reverse_iterator it(_activeOps.rbegin());
            _activeOps.rend() != it; ++it)
        {
            (*it)->OnCameraChanged();
        }
    }

    bool OnMouseDown(const M::Vector2& mouseCoords) {
        for(int i = _activeOps.size() - 1; 0 <= i; --i) {
            Operator* op = _activeOps[i];
            if(op->OnMouseDown(mouseCoords)) {
                UI::OperatorPanel::Instance()->Clear();
                unsigned N = _activeOps.size() - 1 - i;
                for(unsigned j = 0; j < N; ++j) {
                    _activeOps.back()->Finish();
                    _activeOps.pop_back();
                }
                if(i != _activeOps.size() - 1) op->Invoke();
                return true;
            }
        }
        return false;
    }

    bool OnMouseMove(const M::Vector2& mouseCoords) {
        for(int i = _activeOps.size() - 1; 0 <= i; --i) {
            Operator* op = _activeOps[i];
            if(op->OnMouseMove(mouseCoords)) {
                UI::OperatorPanel::Instance()->Clear();
                unsigned N = _activeOps.size() - 1 - i;
                for(unsigned j = 0; j < N; ++j) {
                    _activeOps.back()->Finish();
                    _activeOps.pop_back();
                }
                if(i != _activeOps.size() - 1) op->Invoke();
                return true;
            }
        }
        return false;
    }

    bool OnMouseUp(const M::Vector2& mouseCoords) {
        for(int i = _activeOps.size() - 1; 0 <= i; --i) {
            Operator* op = _activeOps[i];
            if(op->OnMouseUp(mouseCoords)) {
                UI::OperatorPanel::Instance()->Clear();
                unsigned N = _activeOps.size() - 1 - i;
                for(unsigned j = 0; j < N; ++j) {
                    _activeOps.back()->Finish();
                    _activeOps.pop_back();
                }
                if(i != _activeOps.size() - 1) op->Invoke();
                return true;
            }
        }
        return false;
    }
};

extern Operators g_operators;

} // namespace OP