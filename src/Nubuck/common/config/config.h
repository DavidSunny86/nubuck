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
        struct CallbackObject {
            virtual ~CallbackObject(void) { }
            virtual void CVAR_Changed(const std::string& name) = 0;
        };

		typedef void (*CallbackFunction)(const std::string& name);

        template<typename TYPE>
        class Variable : public VarInfo {
        private:
            std::string _name;
            TYPE _val;
            SYS::SpinLock _mtx; // locks observers
            std::vector<CallbackObject*> _cbObjs;
			std::vector<CallbackFunction> _cbFuncs;
        public:
            Variable(const std::string& name, const TYPE& defaultValue) : _name(name), _val(defaultValue) { 
                Config::Instance().Add(this);
            }

            const std::string& Name(void) const override { return _name; }

            void Register(CallbackObject* const cb) { 
                _mtx.Lock();
                _cbObjs.push_back(cb);
                _mtx.Unlock();
            }

            void Unregister(CallbackObject* const cb) {
                _mtx.Lock();
                for(std::vector<CallbackObject*>::iterator cbIt(_cbObjs.begin()); _cbObjs.end() != cbIt; ++cbIt) {
                    if(*cbIt == cb) {
                        _cbObjs.erase(cbIt);
                        break;
                    }
                }
                _mtx.Unlock();
            }

			void Register(CallbackFunction cb) {
				_mtx.Lock();
				_cbFuncs.push_back(cb);
				_mtx.Unlock();
			}

			void Unregister(CallbackFunction cb) {
				_mtx.Lock();
				for(std::vector<CallbackFunction>::iterator cbIt(_cbFuncs.begin()); _cbFuncs.end() != cbIt; ++cbIt) {
					if(*cbIt == cb) {
						_cbFuncs.erase(cbIt);
						break;
					}
				}
				_mtx.Unlock();
			}

            operator TYPE() const { return _val; }

            Variable& operator=(const TYPE& val) {
                _val = val;
                _mtx.Lock();
                for(std::vector<CallbackObject*>::iterator cbIt(_cbObjs.begin()); _cbObjs.end() != cbIt; ++cbIt)
                    if(*cbIt) (*cbIt)->CVAR_Changed(_name);
				for(std::vector<CallbackFunction>::iterator cbIt(_cbFuncs.begin()); _cbFuncs.end() != cbIt; ++cbIt)
					if(*cbIt) (*cbIt)(_name);
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