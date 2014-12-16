#pragma once

#include <assert.h>
#include <queue>
#include <Nubuck\common\common.h>
#include <Nubuck\common\string_helper.h>
#include <Nubuck\system\locks\spinlock.h>
#include <Nubuck\system\locks\condvar.h>
#include <Nubuck\system\locks\semaphore.h>
#include <Nubuck\system\locks\shared_resources.h>

// events are sent and received between threads. a thread queues it's received events
// and processes them in order.
// the sending of an event can be blocking or non-blocking. in the former case the
// sending thread blocks until the receiving thread signals the event has been
// processed.

typedef unsigned eventID_t;
typedef unsigned eventTypeID_t;

NUBUCK_API eventID_t EV_GetNextID();

#define EVENT_TYPE(classname)                                       \
    public:                                                         \
        static eventTypeID_t GetEventTypeID() {                     \
            static int inf;                                         \
            return reinterpret_cast<eventTypeID_t>(&inf);           \
        }                                                           \
        eventTypeID_t GetDynamicEventTypeID() const override {      \
            return GetEventTypeID();                                \
        }                                                           \
        const char* Name() const override {                         \
            return #classname;                                      \
        }                                                           \
        Event* Clone() const override {                             \
            return new classname(*this);                            \
        }

#define EVENT_GENERIC_CLONE(classname) \
    public: Event* Clone() const override { return new classname(*this); }

#define EV_MAX_EVENT_ID UINT_MAX /* invalid event id */

namespace EV {

struct BlockingEvent {
    SYS::SpinLock*          mtx;
    SYS::ConditionVariable* cv;
    bool                    sig;
};

struct Event {
    eventID_t       id0;
    eventID_t       id1;
    BlockingEvent*  block;
    bool            tagged;

    Event() : block(NULL), tagged(false), id1(EV_MAX_EVENT_ID) { }

    static eventTypeID_t GetEventTypeID() { return 0; }

    virtual eventTypeID_t GetDynamicEventTypeID() const { return 0; }
    virtual bool Merge(const Event& other) { return false; }
    virtual const char* Name() const { return "base event"; }
    virtual Event* Clone() const { return new Event(*this); }

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

template<typename T>
struct Arg : Event {
    EVENT_TYPE(Arg)

    T value;

    Arg() { }
    Arg(const T& value) : value(value) { }
};

template<typename T0, typename T1>
struct Args2 : Event {
    EVENT_TYPE(Args2)

    T0 value0;
    T1 value1;

    Args2() { }
    Args2(const T0& value0, const T1& value1)
        : value0(value0), value1(value1)
    { }
};

// links id to type
struct EventDef {
    virtual ~EventDef() { }
    
    virtual eventID_t GetEventID() const = 0;
    virtual eventTypeID_t GetEventTypeID() const = 0;
};

template<typename T_Event>
struct ConcreteEventDef : EventDef {
private:
    eventID_t       _eventID;
    eventTypeID_t   _eventTypeID;
public:
    ConcreteEventDef() {
        _eventID = EV_GetNextID();
        _eventTypeID = T_Event::GetEventTypeID();
    }

    eventID_t GetEventID() const {
        return _eventID;
    }

    eventTypeID_t GetEventTypeID() const {
        return _eventTypeID;
    }

    // NOTE: in case of default argument a referenced to a temporary object is
    // returned. this is fine, however, because the event gets cloned anyways
    const T_Event& Tag(T_Event& event = T_Event(), const eventID_t id1 = EV_MAX_EVENT_ID) const {
        event.id0 = _eventID;
        event.id1 = id1;
        event.tagged = true;
        return event;
    }
};

template<typename T>
struct ConcreteEventDef<Arg<T> > : EventDef {
private:
    eventID_t       _eventID;
    eventTypeID_t   _eventTypeID;
public:
    ConcreteEventDef() {
        _eventID = EV_GetNextID();
        _eventTypeID = Arg<T>::GetEventTypeID();
    }

    eventID_t GetEventID() const {
        return _eventID;
    }

    eventTypeID_t GetEventTypeID() const {
        return _eventTypeID;
    }

    // NOTE: in case of default argument a referenced to a temporary object is
    // returned. this is fine, however, because the event gets cloned anyways
    const Arg<T>& Tag(Arg<T>& event = Arg<T>(), const eventID_t id1 = EV_MAX_EVENT_ID) const {
        event.id0 = _eventID;
        event.id1 = id1;
        event.tagged = true;
        return event;
    }

    Arg<T> Tag(const T& value, const eventID_t id1 = EV_MAX_EVENT_ID) const {
        Arg<T> event(value);
        return Tag(event, id1);
    }
};

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
    typename T_Policy = EventHandlerPolicies::Nonblocking
>
class EventHandler {
protected:
    struct AbstractHandler;
private:
    std::queue<EV::Event*>                          _ev_events;
    SYS::SpinLock                       			_ev_eventsMtx;
    T_Policy                              			_ev_policy;
    std::vector<GEN::Pointer<AbstractHandler> >     _ev_handlers;
protected:
    struct AbstractHandler {
        eventID_t    id0;
        eventID_t    id1;
        virtual void Call(const Event& event) = 0;
    };

    template<typename T_Instance, typename T_Event>
    struct ConcreteHandler : AbstractHandler {
        typedef void (T_Instance::*memfunc_t)(const T_Event& event);
        T_Instance* instance;
        memfunc_t   memfunc;

        void Call(const Event& event) override {
            const T_Event& realEvent = (const T_Event&)event;
            (instance->*memfunc)(realEvent);
        }
    };
public:
    template<typename T_Instance, typename T_Event>
    void AddEventHandler(
        const ConcreteEventDef<T_Event>& eventDef,
        T_Instance* instance,
        typename ConcreteHandler<T_Instance, T_Event>::memfunc_t memfunc,
        const eventID_t id1 = EV_MAX_EVENT_ID)
    {
        assert(instance);
        assert(memfunc);
        assert(eventDef.GetEventTypeID() == T_Event::GetEventTypeID()); // type compatibility
        GEN::Pointer<ConcreteHandler<T_Instance, T_Event> > item(new ConcreteHandler<T_Instance, T_Event>);
        item->id0 = eventDef.GetEventID();
        item->id1 = id1;
        item->instance = instance;
        item->memfunc = memfunc;
        _ev_handlers.push_back(item);
    }
protected:
    unsigned GetEventQueueSize() const { return _ev_events.size(); }

    template<typename T_Instance>
    void _EV_HandleEvents(T_Instance* instance, const char* className) {
        bool done = false;
        while(!done) {
            _ev_policy.WaitEvent();
            Event* event;
            _ev_eventsMtx.Lock();
            if(_ev_events.empty()) done = true;
            else {
                event = _ev_events.front();
                _ev_events.pop();
            }
            _ev_eventsMtx.Unlock();
            if(done) break;
            AbstractHandler *match = NULL, *h = NULL;
            for(unsigned i = 0; i < _ev_handlers.size(); ++i) {
                h = _ev_handlers[i].Raw();
                if(event->id0 == h->id0) {
                    if(
                        event->id1 == h->id1 ||                 // specialized handler
                        (!match && EV_MAX_EVENT_ID == h->id1))  // general handler
                    {
                        match = h;
                    }
                }
            } // forall handlers
            if(match) match->Call(*event);
            else Event_Default(*event, className);
            delete event;
        } /* while(!done) */
    }

    virtual void Event_Default(const Event& event, const char* className) {
        COM_printf("WARNING - unhandled event '%s' (id = '%d') in class '%s'.\n",
            event.Name(), event.id0, className);
    }
public:
    virtual ~EventHandler() { }

    void Send(const Event& event) { // nonblocking
        assert(event.tagged);
        assert(NULL == event.block);
        _ev_eventsMtx.Lock();
        Event* copy = event.Clone();
        _ev_events.push(copy);
        _ev_eventsMtx.Unlock();
        _ev_policy.SignalEvent();
    }

    void SendAndWait(const EV::Event& event) { // blocking
        SYS::SharedResource<SYS::SpinLock> mtx;
        SYS::SharedResource<SYS::ConditionVariable> cv;

        BlockingEvent block;
        block.mtx = &mtx.Resource();
        block.cv = &cv.Resource();
        block.sig = false;
        // event.block = &block;

        _ev_eventsMtx.Lock();
        Event* copy = event.Clone();
        copy->block = &block;
        _ev_events.push(copy);
        _ev_eventsMtx.Unlock();
        _ev_policy.SignalEvent();

        block.mtx->Lock();
        while(!block.sig) block.cv->Wait(*block.mtx);
        block.mtx->Unlock();
    }
}; // class EventHandler

} // namespace EV

#define DECL_HANDLE_EVENTS(CLASS) void HandleEvents(void) { _EV_HandleEvents(this, #CLASS); }