#pragma once

#include <Windows.h>

// crazy token pasting
// cnf. http://stackoverflow.com/questions/1597007/creating-c-macro-with-and-line-token-concatenation-with-positioning-macr/1597129#1597129
#define SYS_PROF_TOK_PASTE0(x, y, z) x ## y ## z
#define SYS_PROF_TOK_PASTE1(x, y, z) SYS_PROF_TOK_PASTE0(x, y, z)

// create identifiers from function name
#define SYS_PROF_SCOPE_DECL_IDENT(name) SYS_PROF_TOK_PASTE1(profScope_, name, __LINE__)
#define SYS_PROF_SAMPLER_IDENT(name) SYS_PROF_TOK_PASTE1(profSampler_, name, __LINE__)

#define SYS_PROFILER_SCOPE(prof, ident, name) \
    static SYS::ProfilerScopeDecl SYS_PROF_SCOPE_DECL_IDENT(ident)(prof, name); \
    const SYS::ProfilerScopeDecl::Sampler& SYS_PROF_SAMPLER_IDENT(ident) = SYS_PROF_SCOPE_DECL_IDENT(ident).GetSampler();

#define SYS_PROFILER_SCOPE_IDENT(prof, ident) SYS_PROFILER_SCOPE(prof, ident, #ident)

#define SYS_PROFILER_SCOPE_NAME(prof, name) SYS_PROFILER_SCOPE(prof, SYS_PROF_TOK_PASTE1(f, unc, __LINE__), name)

#define SYS_PROFILER_SCOPE_AUTO(prof) SYS_PROFILER_SCOPE(prof, SYS_PROF_TOK_PASTE1(f, unc, __LINE__), __FUNCTION__)

namespace SYS {

struct ProfilerScope {
    ProfilerScope* childs;
    ProfilerScope* sibs;
    ProfilerScope* parent;

    const char* name;
    float       total; // total elapsed time
    float       numCalls;

    ProfilerScope();
    explicit ProfilerScope(const char* name);
};

class Profiler {
private:
    enum { CALL_STACK_CAP = 64 };

    ProfilerScope   _root;

    ProfilerScope*  _callStack[CALL_STACK_CAP];
    int             _top;

    LONGLONG        _frequency;
public:
    Profiler();

    void PushScope(ProfilerScope* scope);
    void PopScope();

    ProfilerScope* CurrentScope();

    LONGLONG GetFrequency() const;

    void WriteReport(const char* filename);
};

class ProfilerScopeDecl {
private:
    Profiler&       _prof;
    ProfilerScope   _scope;
public:
    class Sampler {
    private:
        Profiler&       _prof;
        ProfilerScope&  _scope;

        LONGLONG        _start;
    public:
        Sampler(Profiler& prof, ProfilerScope& scope);
        ~Sampler();
    };

    explicit ProfilerScopeDecl(Profiler& prof, const char* name);

    Sampler GetSampler();
};

} // namespace SYS

#include "scoped_profiler_inl.h"