#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include <Nubuck\generic\singleton.h>
#include <Nubuck\system\locks\spinlock.h>

namespace COM {

	class Config : public GEN::Singleton<Config> {
		friend class GEN::Singleton<Config>;
    private:
        struct VarInfo {
            VarInfo* next;
            virtual const std::string& Name(void) const = 0;
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

            const std::string& Name(void) const override { return _name; }
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
        void Add(VarInfo* const varInfo) {
            varInfo->next = _vars;
            _vars = varInfo;
        }

        void DumpVariables(void);
	};

} // namespace COM