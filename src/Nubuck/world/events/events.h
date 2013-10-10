#pragma once

#include <queue>
#include <system\locks\spinlock.h>

// events are sent and received between threads. a thread queues it's received events
// and processes them in order.
// the sending of an event can be blocking or non-blocking. in the former case the
// sending thread blocks until the receiving thread signals the event has been
// processed.

namespace EV {

    struct Event {
        enum { ARGS_SIZE = 64 };
        unsigned    id;
        char        args[ARGS_SIZE]; 
    };

    class DefinitionBase { 
    private:
        unsigned _id;
    public:
        DefinitionBase(void) : _id(0) {
            extern unsigned evIdCnt;
            _id = evIdCnt++;
        }
        unsigned GetID(void) const { return _id; }
    };

    template<typename PARAMS>
    struct Definition : DefinitionBase {
        const char* name;

        Definition(const char* name) : name(name) {
            assert(Event::ARGS_SIZE >= sizeof(PARAMS));
        }

        Event Create(const PARAMS& params) {
            Event event;
            event.id = GetID();
            memcpy(event.args, &params, sizeof(PARAMS));
            return event;
        }

        const PARAMS& GetArgs(const Event& event) {
            assert(event.id == GetID());
            return *(PARAMS*)event.args;
        }
    };

} // namespace EV

#define BEGIN_EVENT_DEF(IDENT)                          \
    namespace EV {                                      \
        struct Params_##IDENT;                          \
        extern Definition<Params_##IDENT> def_##IDENT;  \
        struct Params_##IDENT {

#define END_EVENT_DEF                                   \
        }; /* struct Params */                          \
    } /* namespace EV */

#define ALLOC_EVENT_DEF(IDENT)                          \
    namespace EV {                                      \
        Definition<Params_##IDENT> def_##IDENT(#IDENT); \
    } /* namespace EV */

namespace EV {

template<typename TYPE>
class EventHandler {
protected:
    struct EventHandlerMapItem;
private:
    std::queue<EV::Event>               _ev_events;
    SYS::SpinLock                       _ev_eventsMtx;
    std::vector<EventHandlerMapItem>    _ev_cache;
protected:
    typedef void (TYPE::*eventHandlerFunc_t)(const EV::Event& event);
    struct EventHandlerMapItem {
        EV::DefinitionBase* def;
        eventHandlerFunc_t func;
    };

    void _EV_HandleEvents(TYPE* instance, const char* className, EventHandlerMapItem eventHandlerMap[]) {                               
        static EventHandlerMapItem invIt = { NULL, NULL };
        bool done = false;                                  
        while(!done) {                                      
            EV::Event event;                                
            _ev_eventsMtx.Lock();                           
            if(_ev_events.empty()) done = true;             
            else {                                          
                event = _ev_events.front();                     
                _ev_events.pop();                               
            }                                               
            _ev_eventsMtx.Unlock();                         
            if(done) break;                                 
            if(_ev_cache.size() <= event.id) _ev_cache.resize(event.id + 1, invIt);
            if(!_ev_cache[event.id].def) {
                EventHandlerMapItem* it = eventHandlerMap;
                while(NULL != it->def) {                         
                    if(event.id == it->def->GetID()) {           
                        _ev_cache[event.id] = *it;
                        break;
                    }                                           
                    it++;
                }                                               
            }
            if(_ev_cache[event.id].def) {
                assert(_ev_cache[event.id].func);
                (instance->*(_ev_cache[event.id].func))(event);
            } else {
                common.printf("WARNING - unhandled event with id = '%d' in class '%s'.\n",
                    event.id, className);
            }
        } /* while(!done) */                                
    }
public:
    void Send(const EV::Event& event) {
        _ev_eventsMtx.Lock();
        _ev_events.push(event);
        _ev_eventsMtx.Unlock();
    }
}; // class EventHandler

} // namespace EV

#define DECLARE_EVENT_HANDLER(CLASS)                                    \
private:                                                                \
    static const char*          _ev_className;                          \
    static EventHandlerMapItem  _ev_eventHandlerMap[];                  \
                                                                        \
    void HandleEvents(void) {                                           \
        _EV_HandleEvents(this, _ev_className, _ev_eventHandlerMap);     \
    }

#define BEGIN_EVENT_HANDLER(CLASS)                                      \
    const char* CLASS::_ev_className = #CLASS;                          \
    CLASS::EventHandlerMapItem CLASS::_ev_eventHandlerMap[] = {

#define EVENT_HANDLER(EVENT, HANDLER)   \
        { &EVENT, HANDLER }, 

#define END_EVENT_HANDLER               \
        { NULL, NULL }                  \
    }; /* eventHandlerMap */
