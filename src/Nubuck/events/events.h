#pragma once

#include <assert.h>
#include <queue>
#include <common\common.h>
#include <common\string_hash.h>
#include <system\locks\spinlock.h>
#include <system\locks\condvar.h>
#include <system\locks\semaphore.h>
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

namespace EventHandlerPolicies {

struct Nonblocking {
    void WaitEvent() { }
    void SignalEvent() { }
};

struct Blocking {
    SYS::Semaphore _sem;
    Blocking() : _sem(0) { }
    void WaitEvent() { _sem.Wait(); }
    void SignalEvent() { _sem.Signal(); }
};

} // namespace EventHandlerPolicies

template<
    typename POLICY = EventHandlerPolicies::Nonblocking
>
class EventHandler {
protected:
    struct AbstractHandler;
private:
    std::queue<EV::Event>                           _ev_events;
    SYS::SpinLock                       			_ev_eventsMtx;
    POLICY                              			_ev_policy;
    std::vector<GEN::Pointer<AbstractHandler> >     _ev_handlers;
protected:
    struct AbstractHandler {
        unsigned    eventID;
        virtual void Call(const Event& event) = 0;
    };

    template<typename TYPE>
    struct ConcreteHandler : AbstractHandler {
        typedef void (TYPE::*Memfunc)(const Event& event);
        TYPE*   instance;
        Memfunc memfunc;

        void Call(const EV::Event& event) override {
            (instance->*memfunc)(event);
        }
    };
public:
    template<typename PARAMS, typename TYPE>
    void AddEventHandler(const EventDefinition<PARAMS>& eventDef, TYPE* instance, typename ConcreteHandler<TYPE>::Memfunc memfunc) {
        assert(instance);
        assert(memfunc);
        GEN::Pointer<ConcreteHandler<TYPE> > item(new ConcreteHandler<TYPE>());
        item->eventID = eventDef.GetEventID();
        item->memfunc = memfunc;
        item->instance = instance;
        _ev_handlers.push_back(item);
    }
protected:
    template<typename TYPE>
    void _EV_HandleEvents(TYPE* instance, const char* className) {                               
        bool done = false;                                  
        while(!done) {                                      
            _ev_policy.WaitEvent();
            EV::Event event;                                
            _ev_eventsMtx.Lock();                           
            if(_ev_events.empty()) done = true;             
            else {                                          
                event = _ev_events.front();                     
                _ev_events.pop();                               
            }                                               
            _ev_eventsMtx.Unlock();                         
            if(done) break;                                 
            bool called = false;
            for(unsigned i = 0; i < _ev_handlers.size(); ++i) {
                if(_ev_handlers[i]->eventID == event.id) {
                    _ev_handlers[i]->Call(event);
                    called = true;
                }
            }
            if(!called) Event_Default(event, className);
        } /* while(!done) */                                
    }

    virtual void Event_Default(const EV::Event& event, const char* className) { 
        common.printf("WARNING - unhandled event '%s' (id = '%d') in class '%s'.\n",
            event.name, event.id, className);
    }
public:
    virtual ~EventHandler() { }

    void Send(const EV::Event& event) { // nonblocking
        assert(NULL == event.block);
        _ev_eventsMtx.Lock();
        _ev_events.push(event);
        _ev_eventsMtx.Unlock();
        _ev_policy.SignalEvent();
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
        _ev_policy.SignalEvent();

        block.mtx->Lock();
        while(!block.sig) block.cv->Wait(*block.mtx);
        block.mtx->Unlock();
    }
}; // class EventHandler

} // namespace EV

#define DECL_HANDLE_EVENTS(CLASS) void HandleEvents(void) { _EV_HandleEvents(this, #CLASS); }