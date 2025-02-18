#pragma once

#include <tinyfsm/tinyfsm.hpp>
#include <glm/glm.hpp>
#include "Graphics/Renderer.h"

struct PlayerEvent : tinyfsm::Event
{
    glm::vec3 direction;
    PlayerEvent() {}
    PlayerEvent(const glm::vec4 &dir) : direction(dir) {}
};

class PlayerMachine : public tinyfsm::Fsm<PlayerMachine>
{
public:
    static glm::vec3 direction;
    static float speed;

    void react(tinyfsm::Event const &) {};

    virtual void react(PlayerEvent const &) {};

    virtual void entry(void) {};
    void exit(void) {};
};

class Idle : public PlayerMachine
{
protected:
    void react(PlayerEvent const &) override;
    void entry(void) override;
};

class Moving : public PlayerMachine
{
protected:
    virtual void react(PlayerEvent const &) override;
};

class Walking : public Moving
{
protected:
    void react(PlayerEvent const &) override;
    void entry(void) override;
};

class Running : public Moving
{
protected:
    void react(PlayerEvent const &) override;
    void entry(void) override;
};

template <typename E>
inline void sendPlayerEvent(E const &event)
{
    tinyfsm::FsmList<PlayerMachine>::template dispatch<E>(event);
}