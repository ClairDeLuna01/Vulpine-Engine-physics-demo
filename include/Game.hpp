#pragma once
#include <../Engine/include/App.hpp>
#include <../Engine/include/Physics.hpp>

class Game final : public App
{
private:
    MeshMaterial depthOnlyMaterial;
    MeshMaterial PBR;
    MeshMaterial basic;
    // MeshMaterial basic2;
    MeshMaterial skyboxMaterial;
    MeshMaterial depthOnlyStencilMaterial;
    MeshMaterial PBRstencil;

    PhysicsEngine physicsEngine;

    static bool move_forward;
    static bool move_backward;
    static bool move_left;
    static bool move_right;
    static bool move_down;
    static bool move_up;

    static bool physicsPaused;
    static bool step;

public:
    Game(GLFWwindow *window);
    void init(int paramSample);
    bool userInput(GLFWKeyInfo input);
    void mainloop();
};