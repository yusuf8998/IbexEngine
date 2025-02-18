#include "PlayerMachine.h"
#include <Graphics/InputManager/InputManager.h>
#include <Engine/Camera.h>

FSM_INITIAL_STATE(PlayerMachine, Idle);

glm::vec3 PlayerMachine::direction = glm::vec3(0, 0, -1);
float PlayerMachine::speed;

void Idle::react(PlayerEvent const &e)
{
    direction = e.direction;
    
    if (glm::length(e.direction) <= 0.125f)
        return;

    if (InputManager::instance().getAxis("Sprint") > 0.f)
        transit<Running>();
    else
        transit<Walking>();
}

void Idle::entry(void)
{
    speed = 0.f;
}

void Moving::react(PlayerEvent const &e)
{
    direction = e.direction;
    auto &renderer = Renderer::instance();

    if (glm::length(e.direction) <= 0.125f)
    {
        transit<Idle>();
        return;
    }

    mainCamera.position += glm::normalize(direction) * speed * renderer.getDeltaTime();
}

void Walking::react(PlayerEvent const &e)
{
    Moving::react(e);
    if (InputManager::instance().getAxis("Sprint") > 0.f)
        transit<Running>();
}

void Walking::entry(void)
{
    speed = 1.f;
}

void Running::react(PlayerEvent const &e)
{
    Moving::react(e);
    if (InputManager::instance().getAxis("Sprint") <= 0.f)
        transit<Walking>();
}

void Running::entry(void)
{
    speed = 5.f;
}