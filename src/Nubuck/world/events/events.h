#pragma once

#include <queue>
#include <system\locks\spinlock.h>

// events are sent and received between threads. a thread queues it's received events
// and processes them in order.
// the sending of an event can be blocking or non-blocking. in the former case the
// sending thread blocks until the receiving thread signals the event has been
// processed.

#define EV_ARGS_SIZE 64

namespace EV {

    struct Event {
        unsigned    id;
        char        args[EV_ARGS_SIZE]; 
    };

    struct EventDef { virtual unsigned GetID(void) const = 0; };

    template<typename PARAMS>
    class Definition : public EventDef {
    private:
        unsigned _id;
    public:
        Definition(void) : _id(0) {
            extern unsigned evIdCnt;
            assert(EV_ARGS_SIZE >= sizeof(PARAMS));
            _id = ++evIdCnt;
        }

        unsigned GetID(void) const override { return _id; }

        Event Create(const PARAMS& params) {
            Event event;
            event.id = _id;
            memcpy(event.args, &params, sizeof(PARAMS));
            return event;
        }

        const PARAMS& GetArgs(const Event& event) {
            assert(event.id == _id);
            return *(PARAMS*)event.args;
        }
    };

} // namespace EV

#define BEGIN_EVENT_DECL(IDENT) \
    namespace EV { \
        struct Params_##IDENT; \
        extern Definition<Params_##IDENT> def_##IDENT; \
        struct Params_##IDENT {

#define END_EVENT_DECL \
        }; /* struct Params */ \
    } /* namespace EV */

#define ALLOC_EVENT_DECL(IDENT)                 \
    namespace EV {                              \
        Definition<Params_##IDENT> def_##IDENT; \
    } /* namespace EV */

template<typename TYPE>
class EventHandler {
protected:
    struct EventHandlerMapItem;
private:
    std::queue<EV::Event> _ev_events;
    SYS::SpinLock _ev_eventsMtx;
    std::vector<EventHandlerMapItem> _ev_cache;
protected:
    typedef void (TYPE::*eventHandlerFunc_t)(const EV::Event& event);
    struct EventHandlerMapItem {
        EV::EventDef* def;
        eventHandlerFunc_t func;
    };

    void _Send(const EV::Event& event) {
        _ev_eventsMtx.Lock();
        _ev_events.push(event);
        _ev_eventsMtx.Unlock();
    }

    void _EV_HandleEvents(TYPE* instance, EventHandlerMapItem eventHandlerMap[]) {                               
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
            EventHandlerMapItem inv = { NULL, NULL };
            if(_ev_cache.size() <= event.id) _ev_cache.resize(event.id + 1, inv);
            if(NULL != _ev_cache[event.id].def) {
                printf("from cache\n");
                (instance->*(_ev_cache[event.id].func))(event);
            }
            else {
            unsigned i = 0;                                 
            EventHandlerMapItem it = eventHandlerMap[i];
            while(NULL != it.def) {                         
                if(event.id == it.def->GetID()) {           
                    printf("searching\n");
                    _ev_cache[event.id] = it;
                    (instance->*(it.func))(event);              
                }                                           
                i++;                                        
                it = eventHandlerMap[i];                
            }                                               
            }
        } /* while(!done) */                                
    }
};

#define DECLARE_EVENT_HANDLER(CLASS)                                    \
    static EventHandlerMapItem _ev_eventHandlerMap[];                   \
                                                                        \
    void HandleEvents(void) {                                           \
        _EV_HandleEvents(this, _ev_eventHandlerMap);                    \
    }

/*
#define DECLARE_EVENT_HANDLER(CLASS)                                    \
    typedef void (CLASS::*eventHandlerFunc_t)(const EV::Event& event);  \
    struct EventHandlerMapItem {                                        \
        EV::EventDef*       def;                                        \
        eventHandlerFunc_t  func;                           	        \
    };                                                      	        \
    static EventHandlerMapItem _ev_eventHandlerMap[];                   \
                                                            			\
    std::queue<EV::Event>   _ev_events;                       			\
    SYS::SpinLock           _ev_eventsMtx;                    			\
                                                            			\
    void Send(const EV::Event& event) {                     			\
        _ev_eventsMtx.Lock();                                 			\
    	_ev_events.push(event);                                   		\
    	_ev_eventsMtx.Unlock();                               	        \
    }                                                       			\
                                                            	        \
    void HandleEvents(void) {                               			\
        bool done = false;                                  			\
        while(!done) {                                      			\
            EV::Event event;                                			\
            _ev_eventsMtx.Lock();                             			\
            if(_ev_events.empty()) done = true;               			\
            else {                                          			\
            event = _ev_events.front();                       			\
            _ev_events.pop();                                 			\
            }                                               			\
            _ev_eventsMtx.Unlock();                           			\
            if(done) break;                                 			\
            unsigned i = 0;                                 	        \
            EventHandlerMapItem it = _ev_eventHandlerMap[i];   			\
            while(NULL != it.def) {                         			\
                if(event.id == it.def->GetID()) {           			\
                    (this->*(it.func))(event);              			\
                }                                           			\
                i++;                                        			\
                it = _ev_eventHandlerMap[i];                   			\
            }                                               			\
        } /* while(!done) *a/                                			\
    };
    */

#define BEGIN_EVENT_HANDLER(CLASS) \
    CLASS::EventHandlerMapItem CLASS::_ev_eventHandlerMap[] = {

#define EVENT_HANDLER(EVENT, HANDLER) \
        { &EVENT, HANDLER }, 

#define END_EVENT_HANDLER \
        { NULL, NULL } \
    }; /* eventHandlerMap */
