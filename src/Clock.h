#pragma once

#include <functional>

struct Clock
{
private:
    float currentTime;
    std::function<void(void)> action;
    std::function<float(float, float)> updateEvent;

public:
    const float maxTime;

    Clock(float _maxTime, const std::function<void(void)> &_action)
        : maxTime(_maxTime), currentTime(0.f), action(_action) {}
    Clock(float _maxTime, const std::function<void(void)> &_action, const std::function<float(float, float)> &_updateEvent)
        : maxTime(_maxTime), currentTime(0.f), action(_action), updateEvent(_updateEvent) {}

    inline void update(float dt)
    {
        if (updateEvent)
            currentTime = updateEvent(currentTime, dt);
        else
            currentTime += dt;
        if (currentTime < maxTime)
            return;
        currentTime = 0.f;
        action();
    }

    inline void reset()
    {
        currentTime = 0.f;
    }

    inline float getCurrentTime() const { return currentTime; }
};