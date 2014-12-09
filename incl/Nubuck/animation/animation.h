#pragma once

#include <Nubuck\nubuck_api.h>

namespace EV { struct Event; }

namespace A {

class NUBUCK_API Animation {
public:
    struct AnimatorLink {
        Animation *prev, *next;
    } animatorLink;

    struct SubjectLink {
        Animation* next;
    } subjectLink;

    typedef bool (*eventFilter_t)(const EV::Event& event);
private:
    struct AnimMode {
        enum Enum {
            PLAY_FOR_DURATION = 0,
            PLAY_UNTIL_EVENT,
            PLAY_UNTIL_DONE
        };
    };

    AnimMode::Enum  _mode;
    bool            _isDone;
    bool            _isMoveDone;

    float           _time;
    float           _duration;
    eventFilter_t   _eventFilter;
protected:
    virtual bool Animate(float secsPassed) = 0;
public:
    Animation();
    virtual ~Animation();

    bool IsDone() const;

    void Move(float secsPassed);
    void FilterEvent(const EV::Event& event);

    void PlayFor(float duration);
    void PlayUntil(eventFilter_t filter);
    void PlayUntilIsDone();
};

// stock event filters

bool IsWidgetEvent(const EV::Event& event);

} // namespace A