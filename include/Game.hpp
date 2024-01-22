#pragma once
#include <../Engine/include/App.hpp>
#include <../Engine/include/PhysicsEngine.hpp>

class Game final : public App
{
private:
    MeshMaterial depthOnlyMaterial;
    MeshMaterial PBR;
    MeshMaterial basic;
    MeshMaterial skyboxMaterial;
    MeshMaterial depthOnlyStencilMaterial;
    MeshMaterial PBRstencil;

    PhysicsEngine physicsEngine;

    static bool move_forward;
    static bool move_backward;
    static bool move_left;
    static bool move_right;

    static bool physicsPaused;
    static bool step;

public:
    Game(GLFWwindow *window);
    void init(int paramSample);
    bool userInput(GLFWKeyInfo input);
    void mainloop();
};