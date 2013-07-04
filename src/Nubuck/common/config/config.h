#pragma once

#include <stdio.h>
#include <string>
#include <vector>

#include <generic\singleton.h>
#include <system\locks\spinlock.h>

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
        struct Observer {
            virtual ~Observer(void) { }
            virtual void CVAR_Changed(const std::string& name) = 0;
        };

        template<typename TYPE>
        class Variable : public VarInfo {
        private:
            std::string _name;
            TYPE _val;
            SYS::SpinLock _mtx; // locks observers
            std::vector<Observer*> _obs;
        public:
            Variable(const std::string& name, const TYPE& defaultValue) : _name(name), _val(defaultValue), _obs(NULL) { 
                Config::Instance().Add(this);
            }

            const std::string& Name(void) const override { return _name; }

            void Register(Observer* const obs) { 
                _mtx.Lock();
                _obs.push_back(obs);
                _mtx.Unlock();
            }

            void Unregister(Observer* const obs) {
                _mtx.Lock();
                for(std::vector<Observer*>::iterator obsIt(_obs.begin()); _obs.end() != obsIt; ++obsIt) {
                    if(*obsIt == obs) {
                        _obs.erase(obsIt);
                        break;
                    }
                }
                _mtx.Unlock();
            }

            operator TYPE() const { return _val; }

            Variable& operator=(const TYPE& val) {
                _val = val;
                _mtx.Lock();
                for(std::vector<Observer*>::iterator obsIt(_obs.begin()); _obs.end() != obsIt; ++obsIt)
                    if(*obsIt) (*obsIt)->CVAR_Changed(_name);
                _mtx.Unlock();
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