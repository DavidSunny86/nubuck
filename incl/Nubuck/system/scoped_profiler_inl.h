#include <stack>
#include "scoped_profiler.h"

namespace SYS {

//==================================================
//=== ProfilerScope
//==================================================

ProfilerScope::ProfilerScope()
    : childs(NULL)
    , sibs(NULL)
    , parent(NULL)
    , name("unknown")
    , total(0.0f)
    , numCalls(0)
{ }

ProfilerScope::ProfilerScope(const char* name)
    : childs(NULL)
    , sibs(NULL)
    , parent(NULL)
    , name(name)
    , total(0.0f)
    , numCalls(0)
{ }

//==================================================
//=== Profiler
//==================================================

Profiler::Profiler() : _top(0) {
    PushScope(&_root);

    if(!QueryPerformanceFrequency((LARGE_INTEGER*)&_frequency)) {
        // throw PerformanceCounterException();
    }
}

void Profiler::PushScope(ProfilerScope* scope) {
    _callStack[_top++] = scope;
}

void Profiler::PopScope() {
    _top--;
}

ProfilerScope* Profiler::CurrentScope() {
    return _callStack[_top - 1];
}

inline LONGLONG Profiler::GetFrequency() const {
    return _frequency;
}

struct DFS_Node {
    ProfilerScope*  scope;
    ProfilerScope*  nextChild;

    explicit DFS_Node(ProfilerScope* scope)
        : scope(scope)
        , nextChild(scope->childs)
    { }
};

void Profiler::WriteReport(const char* filename) {
    FILE* file = fopen(filename, "w");
    if(!file) {
        // ...
        return;
    }

    // depth-first search

    std::stack<DFS_Node> dfs;
    dfs.push(DFS_Node(&_root));

    int nesting = 0;

    while(!dfs.empty()) {
        DFS_Node& node = dfs.top();

        if(node.scope && &_root != node.scope) {
            for(int i = 0; i < nesting; ++i)
                fprintf(file, "....");

            float avg = node.scope->total / node.scope->numCalls;

            float per = 1.0f;
            if(node.scope->parent && &_root != node.scope->parent) {
                per = node.scope->total / node.scope->parent->total;
            }

            fprintf(file, " total: %.6f avg: %.6f %%: %.4f name: %s\n",
                node.scope->total,
                avg,
                per,
                node.scope->name);

            node.scope = NULL;
        }

        if(node.nextChild) {
            dfs.push(DFS_Node(node.nextChild));
            node.nextChild = node.nextChild->sibs;
            nesting++;
        } else {
            dfs.pop();
            nesting--;
        }
    }

    fclose(file);
}

//==================================================
//=== ProfilerScopeDecl::Sampler
//==================================================

ProfilerScopeDecl::Sampler::Sampler(Profiler& prof, ProfilerScope& scope)
    : _prof(prof)
    , _scope(scope)
{
    prof.PushScope(&_scope);

	QueryPerformanceCounter((LARGE_INTEGER*)&_start);
}

ProfilerScopeDecl::Sampler::~Sampler() {
    LONGLONG stop;
    QueryPerformanceCounter((LARGE_INTEGER*)&stop);
    _scope.total += (float)(stop - _start) / _prof.GetFrequency();
    _scope.numCalls++;

    _prof.PopScope();
}

//==================================================
//=== ProfilerScopeDecl
//==================================================

ProfilerScopeDecl::ProfilerScopeDecl(Profiler& prof, const char* name)
    : _prof(prof)
    , _scope(name)
{
    ProfilerScope* parent = prof.CurrentScope();

    // add self as child
    _scope.sibs = parent->childs;
    parent->childs = &_scope;

    _scope.parent = parent;
}

ProfilerScopeDecl::Sampler ProfilerScopeDecl::GetSampler() {
    return Sampler(_prof, _scope);
}

} // namespace SYS