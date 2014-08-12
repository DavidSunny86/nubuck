#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include <Nubuck\generic\singleton.h>
#include <Nubuck\system\locks\spinlock.h>

// TODO: template nonsense. maybe variant would be better?

class QTextStream;

namespace COM {

namespace Impl {

bool Parse(const std::string& str, int& val);
bool Parse(const std::string& str, float& val);

void SPrint(char* buffer, unsigned bufSz, int val);
void SPrint(char* buffer, unsigned bufSz, float val);

template<typename TYPE> struct TypeName { };
template<> struct TypeName<int>     { static const char* CStr() { return "int"; } };
template<> struct TypeName<float>   { static const char* CStr() { return "float"; } };

} // namespace Impl

class Config : public GEN::Singleton<Config> {
    friend class GEN::Singleton<Config>;
private:
    struct VarInfo {
        VarInfo* next;
        virtual const char*         TypeString() const = 0;
        virtual const std::string&  Name(void) const = 0;
        virtual bool                Parse(const std::string& str) = 0;
        virtual void                SPrint(char* buffer, unsigned bufSz) = 0;
        // TODO: Serialize, Deserialize
    };
public:
    template<typename TYPE>
    class Variable : public VarInfo {
    private:
        std::string _name;
        TYPE        _val;
    public:
        Variable(const std::string& name, const TYPE& defaultValue) : _name(name), _val(defaultValue) {
            Config::Instance().Add(this);
        }

        const char* TypeString() const override { return Impl::TypeName<TYPE>::CStr(); }

        const std::string& Name(void) const override { return _name; }

        bool Parse(const std::string& str) override {
            TYPE val;
            if(Impl::Parse(str, val)) {
                _val = val;
                return true;
            } else {
                common.printf("WARNING - unable to parse string '%s' as value of type '%s'\n",
                    str.c_str(), Impl::TypeName<TYPE>::CStr());
                return false;
            }
        }

        void SPrint(char* buffer, unsigned bufSz) override {
            Impl::SPrint(buffer, bufSz, _val);
        }

        operator TYPE() const { return _val; }

        Variable& operator=(const TYPE& val) {
            _val = val;
            return *this;
        }
    };
private:
    VarInfo* _vars;

    Config(void) : _vars(NULL) { }
public:
    typedef VarInfo* variable_t;

    void Add(VarInfo* const varInfo) {
        varInfo->next = _vars;
        _vars = varInfo;
    }

    void DumpVariables(void);

    variable_t  FirstVariable();
    variable_t  NextVariable(variable_t var);

    const char*         TypeString(variable_t var);
    const std::string&  Name(variable_t var);
    std::string         ValueString(variable_t var);
    bool                Parse(variable_t var, const std::string& str);
};

void CMD_ListVariables(QTextStream& cout, const char*);
void CMD_SetVariable(QTextStream& cout, const char*);

} // namespace COM