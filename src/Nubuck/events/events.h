#pragma once

#include <assert.h>
#include <queue>
#include <common\common.h>
#include <common\string_hash.h>
#include <system\locks\spinlock.h>
#include <system\locks\condvar.h>
#include <system\locks\shared_resources.h>

// events are sent and received between threads. a thread queues it's received events
// and processes them in order.
// the sending of an event can be blocking or non-blocking. in the former case the
// sending thread blocks until the receiving thread signals the event has been
// processed.

namespace EV {

struct BlockingEvent {
    SYS::SpinLock*          mtx;
    SYS::ConditionVariable* cv;
    bool                    sig;
};

struct Event {
    enum { ARGS_SIZE = 68 };
    unsigned        id;
    const char*     name;
    BlockingEvent*  block;
    char            args[ARGS_SIZE]; 

    void Accept() const {
        if(block) {
            assert(block->mtx);
            assert(block->cv);
            block->mtx->Lock();
            block->sig = true;
            block->mtx->Unlock();
            block->cv->Signal();
        }
    }
};

} // namespace EV

namespace EV {

template<typename PARAMS>
class EventDefinition {
private:
    const char* _eventName;
    unsigned    _eventID;
public:
    EventDefinition(const char* name) : _eventName(name), _eventID(COM::StringHash(name)) { }

    unsigned GetEventID() const { return _eventID; }

    Event Create(const PARAMS& params) {
        Event event;
        event.block = NULL;
        event.id = _eventID;
        event.name = _eventName;
        memcpy(event.args, &params, sizeof(PARAMS));
        return event;
    }

    const PARAMS& GetArgs(const Event& event) {
        assert(event.id == _eventID);
        return *(PARAMS*)event.args;
    }
};

} // namespace EV

#define BEGIN_EVENT_DEF(IDENT)                                      \
    namespace EV {              									\
        struct Params_##IDENT;                                      \
        static EventDefinition<Params_##IDENT> def_##IDENT(#IDENT); \
        struct Params_##IDENT {

#define END_EVENT_DEF                                               \
        }; /* struct Params_##IDENT */                              \
    } /* namespace EV */

namespace EV {

template<typename TYPE>
class EventHandler {
public:
    struct EventHandlerMapItem;
private:
    std::queue<EV::Event>               _ev_events;
    SYS::SpinLock                       _ev_eventsMtx;
public:
    typedef void (TYPE::*eventHandlerFunc_t)(const EV::Event& event);
    struct EventHandlerMapItem {
        bool                valid;
        unsigned            eventID;
        eventHandlerFunc_t  func;
    };

    void _EV_HandleEvents(TYPE* instance, const char* className, EventHandlerMapItem eventHandlerMap[]) {                               
        EventHandlerMapItem invIt = { false, 0, NULL };
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
            EventHandlerMapItem* it = eventHandlerMap;
            eventHandlerFunc_t func = NULL;
            while(it->valid) {                         
                if(event.id == it->eventID) {           
                    func = it->func;
                    break;
                }                                           
                it++;
            }                                               
            if(func) (instance->*(func))(event);
            else {
                common.printf("WARNING - unhandled event '%s' (id = '%d') in class '%s'.\n",
                    event.name, event.id, className);
            }
        } /* while(!done) */                                
    }
public:
    void Send(const EV::Event& event) { // nonblocking
        assert(NULL == event.block);
        _ev_eventsMtx.Lock();
        _ev_events.push(event);
        _ev_eventsMtx.Unlock();
    }

    void SendAndWait(EV::Event event) { // blocking
        SYS::SharedResource<SYS::SpinLock> mtx;
        SYS::SharedResource<SYS::ConditionVariable> cv;

        BlockingEvent block;
        block.mtx = &mtx.Resource();
        block.cv = &cv.Resource();
        block.sig = false;
        event.block = &block;

        _ev_eventsMtx.Lock();
        _ev_events.push(event);
        _ev_eventsMtx.Unlock();

        block.mtx->Lock();
        while(!block.sig) block.cv->Wait(*block.mtx);
        block.mtx->Unlock();
    }
}; // class EventHandler

} // namespace EV

#define DECLARE_EVENT_HANDLER(CLASS)                                                \
private:                                                                			\
    static const char*          _ev_className;                          			\
    static EV::EventHandler<CLASS>::EventHandlerMapItem  _ev_eventHandlerMap[];     \
    EV::EventHandler<CLASS>         _ev_handler;                            		\
                                                                        			\
    void HandleEvents(void) {                                           			\
        _ev_handler._EV_HandleEvents(this, _ev_className, _ev_eventHandlerMap);     \
    }                                                                               \
    public: void Send(const EV::Event& event) { _ev_handler.Send(event); }          \
    void SendAndWait(const EV::Event& event) { _ev_handler.SendAndWait(event); }

#define BEGIN_EVENT_HANDLER(CLASS)                                                  \
    const char* CLASS::_ev_className = #CLASS;                                      \
    EV::EventHandler<CLASS>::EventHandlerMapItem CLASS::_ev_eventHandlerMap[] = {

#define EVENT_HANDLER(EVENTDEF, HANDLER)   \
        { true, EVENTDEF.GetEventID(), HANDLER }, 

#define END_EVENT_HANDLER               \
        { false, 0, NULL }              \
    }; /* eventHandlerMap */
