#include <Nubuck\events\core_events.h>
#include <nubuck_private.h>
#include <Nubuck\animation\animation.h>
#include <Nubuck\animation\animator.h>

#include <UI\window_events.h>
#include <world\world_events.h>

namespace A {

float Animation::GetTime() const {
    return _time;
}

float Animation::GetSecsPassed() const {
    return _secsPassed;
}

Animation::Animation()
    : _isDone(true) // animation starts paused
    , _isMoveDone(false)
{
    g_animator.LinkAnimation(this);
}

Animation::~Animation() {
    g_animator.UnlinkAnimation(this);
}

bool Animation::IsDone() const { return _isDone; }

void Animation::Move(float secsPassed) {
    if(!_isMoveDone) {
        _secsPassed = secsPassed;
        _isMoveDone = Animate();

        if(AnimMode::PLAY_UNTIL_DONE == _mode && _isMoveDone) {
            _isDone = true;
        }
    }

    _time += secsPassed;

    if(AnimMode::PLAY_FOR_DURATION == _mode && _duration <= _time) {
        _isDone = true;
    }
}

void Animation::FilterEvent(const EV::Event& event) {
    if(AnimMode::PLAY_UNTIL_EVENT == _mode) {
        assert(_eventFilter);
        if(_eventFilter(event)) {
            _isDone = true;
        }
    }
}

void Animation::PlayFor(float duration) {
    _time = 0.0f;
    _duration = duration;
    _mode = AnimMode::PLAY_FOR_DURATION;
    _isDone = false;
}

void Animation::PlayUntil(eventFilter_t eventFilter) {
    _time = 0.0f;
    _eventFilter = eventFilter;
    _mode = AnimMode::PLAY_UNTIL_EVENT;
    _isDone = false;
}

void Animation::PlayUntilIsDone() {
    _time = 0.0f;
    _mode = AnimMode::PLAY_UNTIL_DONE;
    _isDone = false;
}

// stock event filters

bool IsWidgetEvent(const EV::Event& event) {
    if(ev_key.GetEventID() == event.id) return false;
    if(ev_mouse.GetEventID() == event.id) return false;
    if(ev_resize.GetEventID() == event.id) return false;
    if(ev_w_selectionChanged.GetEventID() == event.id) return false;
    // TODO ... event tags would be nice

    return true;
}

} // namespace A