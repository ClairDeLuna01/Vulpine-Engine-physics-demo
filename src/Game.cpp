#include <Game.hpp>
#include <../Engine/include/Globals.hpp>
#include <GameObject.hpp>
#include <CompilingOptions.hpp>
#include <MathsUtils.hpp>
#include <Fonts.hpp>
#include <FastUI.hpp>
#include <Helpers.hpp>
#include <random>

void printm4(const mat4 &m)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            std::cout << m[i][j] << "\t";
        std::cout << "\n";
    }
};

Game::Game(GLFWwindow *window) : App(window) {}

bool Game::move_forward = false;
bool Game::move_backward = false;
bool Game::move_left = false;
bool Game::move_right = false;
bool Game::move_down = false;
bool Game::move_up = false;

bool Game::physicsPaused = false;
bool Game::step = false;

void Game::init(int paramSample)
{
    App::init();

    ambientLight = vec3(0.1);

    finalProcessingStage = ShaderProgram(
        "shader/post-process/final composing.frag",
        "shader/post-process/basic.vert",
        "",
        globals.standartShaderUniform2D());

    finalProcessingStage.addUniform(ShaderUniform(Bloom.getIsEnableAddr(), 10));

    camera.init(radians(70.0f), globals.windowWidth(), globals.windowHeight(), 0.1f, 1E4f);
    camera.setMouseFollow(false);
    camera.setPosition(vec3(0, 1, 0));
    camera.setDirection(vec3(1, 0, 0));

    /* Loading Materials */
    depthOnlyMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnly.frag",
            "shader/foward/basic.vert",
            ""));

    depthOnlyStencilMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/depthOnlyStencil.frag",
            "shader/foward/basic.vert",
            ""));

    PBR = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    basic = MeshMaterial(
        new ShaderProgram(
            "shader/foward/basic.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    // basic2 = MeshMaterial(
    //     new ShaderProgram(
    //         "shader/foward/basic.frag",
    //         "shader/foward/basic.vert",
    //         "",
    //         globals.standartShaderUniform3D()));

    // basic2->addUniform(ShaderUniform(vec3(0, 1.0, 0), 20));

    PBRstencil = MeshMaterial(
        new ShaderProgram(
            "shader/foward/PBR.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    skyboxMaterial = MeshMaterial(
        new ShaderProgram(
            "shader/foward/skybox.frag",
            "shader/foward/basic.vert",
            "",
            globals.standartShaderUniform3D()));

    PBRstencil.depthOnly = depthOnlyStencilMaterial;
    scene.depthOnlyMaterial = depthOnlyMaterial;

    globals.fpsLimiter.activate();
    globals.fpsLimiter.freq = 144.f;
    glfwSwapInterval(0);
}

bool Game::userInput(GLFWKeyInfo input)
{
    if (baseInput(input))
        return true;

    if (input.action == GLFW_PRESS)
    {
        switch (input.key)
        {
        case GLFW_KEY_ESCAPE:
            state = quit;
            break;

        case GLFW_KEY_F2:
            globals.currentCamera->toggleMouseFollow();
            break;

        case GLFW_KEY_1:
            Bloom.toggle();
            break;
        case GLFW_KEY_2:
            SSAO.toggle();
            break;

        case GLFW_KEY_F5:
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            finalProcessingStage.reset();
            Bloom.getShader().reset();
            SSAO.getShader().reset();
            depthOnlyMaterial->reset();
            PBR->reset();
            basic->reset();
            // basic2->reset();
            skyboxMaterial->reset();
            break;

        case GLFW_KEY_W:
            move_forward = true;
            break;

        case GLFW_KEY_S:
            move_backward = true;
            break;

        case GLFW_KEY_A:
            move_left = true;
            break;

        case GLFW_KEY_D:
            move_right = true;
            break;

        case GLFW_KEY_Q:
            move_down = true;
            break;

        case GLFW_KEY_E:
            move_up = true;
            break;

        case GLFW_KEY_KP_DECIMAL:
            step = true;
            break;

        case GLFW_KEY_SPACE:
            physicsPaused = !physicsPaused;
            break;

        default:
            break;
        }
    }

    if (input.action == GLFW_RELEASE)
    {
        switch (input.key)
        {
        case GLFW_KEY_W:
            move_forward = false;
            break;

        case GLFW_KEY_S:
            move_backward = false;
            break;

        case GLFW_KEY_A:
            move_left = false;
            break;

        case GLFW_KEY_D:
            move_right = false;
            break;

        case GLFW_KEY_Q:
            move_down = false;
            break;

        case GLFW_KEY_E:
            move_up = false;
            break;

        default:
            break;
        }
    }

    return true;
};

void Game::mainloop()
{
    /* Loading Models and setting up the scene */
    ModelRef skybox = newModel(skyboxMaterial);
    skybox->loadFromFolder("ressources/models/skybox/", true, false);
    // skybox->invertFaces = true;
    skybox->depthWrite = true;
    skybox->state.frustumCulled = false;
    skybox->state.scaleScalar(1E6);
    scene.add(skybox);

    ModelRef cube = newModel(basic);
    cube->state.scale = vec3(10.0f, 0.5f, 6.0f);
    cube->loadFromFolder("ressources/models/cube/", false, false);
    vec3 c2 = vec3(0, 1.0, 0);
    cube->uniforms.add(ShaderUniform(&c2, 20));
    scene.add(cube);

    const std::vector<vec3> vertices{
        {-1.22269,-1.24611,-1},
        {1.71624,0.7437,-1},
        {1.26405,1.95985,0.06473},
        {0.32781,-0.71218,1.84951},
        {-1.48494,0.84139,1}
    };
    PolyhedronHelperRef convex(new PolyhedronHelper(vertices));
    //scene.add(convex);

    SceneDirectionalLight sun = newDirectionLight(
        DirectionLight()
            .setColor(vec3(143, 107, 71) / vec3(255))
            .setDirection(normalize(vec3(-0.454528, -0.707103, 0.541673)))
            .setIntensity(20.0));
    sun->cameraResolution = vec2(2048);
    sun->shadowCameraSize = vec2(90, 90);
    sun->activateShadows();
    scene.add(sun);

    //AABBCollider aabbCollider = AABBCollider(vec3(1.0f, 1.0f, 1.0f));
    AABBCollider aabbCollider = AABBCollider(vec3(10.0f, 0.5f, 6.0f));
    //OBBCollider obbCollider2 = OBBCollider(vec3(1.0f, 1.0f, 1.0f));
    SphereCollider sphereCollider = SphereCollider(1.0f);
    //SphereCollider sphereCollider2 = SphereCollider(1.0f);
    AABBCollider triggerBoxCollider = AABBCollider(vec3(20000000.0f, 10.0f, 20000000.0f));
    //ConvexCollider convexCollider = ConvexCollider(vertices);

    const int layer0 = 0;

    ForceField::Ref gravity = ForceField::global(ForceField::uniformForceField({0.0f, -G, 0.0f}));
    physicsEngine.addForceField(gravity);

    /*ForceField::Ref pull = ForceField::make(
        &sphereCollider,
        vec3(-5.0f, -5.0f, 10.0f),
        identity<quat>(),
        ForceField::pointForceField({-5.0f, -5.0f, 10.0f}, 200.0f, 10.0f)
    );
    physicsEngine.addForceField(pull);*/

    Trigger::Ref trigger = Trigger::make(
        &triggerBoxCollider,
        vec3(0.0f, -28.0f, 0.0f)
    );
    trigger->registerOnEnterEvent([](RigidBody::Ref& rigidBody, float delta) {
        rigidBody->setPosition(vec3(-0.1f * rigidBody->getPosition().x, 45.0f, -0.1f * rigidBody->getPosition().z));
        rigidBody->setVelocity(rigidBody->getVelocity() * 0.1f);
    });
    physicsEngine.addTrigger(trigger);

    constexpr int nSpheres = 800;

    RigidBody::Ref CubeBody = RigidBody::make(
        &aabbCollider,
        -1.0f,
        PhysicsMaterial(0.0f, 0.85f, 0.0f, 0.0f),
        layer0,
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f)
    );

    physicsEngine.addRigidBody(CubeBody);

    GameObject CubeGameObject(newObjectGroup(), CubeBody);
    CubeGameObject.getGroup()->add(cube);

    std::vector<GameObject> sphereGameObjects;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    std::uniform_real_distribution<double> dis2(-50.0, 50.0);

    for (int i = 0; i < nSpheres; ++i) {
        RigidBody::Ref sphereBody = RigidBody::make(
            &sphereCollider,
            1.0f,
            PhysicsMaterial(0.0f, dis(gen) / 2.0f, 0.0f, 0.0f),
            layer0,
            vec3(1.0f + 0.02f * i, 8.0f + 2.0f, -nSpheres * 0.5f + i),
            vec3(dis2(gen), dis(gen), dis2(gen)),
            vec3(0.0f, 0.0f, 0.0f)
        );
        physicsEngine.addRigidBody(sphereBody);

        SphereHelperRef sphere(new SphereHelper({dis(gen), dis(gen), dis(gen)}, 1.0f));
        scene.add(sphere);

        GameObject sphereGameObject(newObjectGroup(), sphereBody);
        sphereGameObject.getGroup()->add(sphere);
        sphereGameObjects.push_back(sphereGameObject);
    }    

    globals.currentCamera->setPosition(vec3(-15, 0, 0));
    globals.currentCamera->lookAt(vec3(0, 0, 0));

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(3.0);

    /* Setting up the UI */
    FontRef font(new FontUFT8);
    font->readCSV("ressources/fonts/Roboto/out.csv");
    font->setAtlas(Texture2D().loadFromFileKTX("ressources/fonts/Roboto/out.ktx"));
    MeshMaterial defaultFontMaterial(
        new ShaderProgram(
            "shader/2D/sprite.frag",
            "shader/2D/sprite.vert",
            "",
            globals.standartShaderUniform2D()));

    MeshMaterial defaultSUIMaterial(
        new ShaderProgram(
            "shader/2D/fastui.frag",
            "shader/2D/fastui.vert",
            "",
            globals.standartShaderUniform2D()));

    SimpleUiTileBatchRef uiBatch(new SimpleUiTileBatch);
    uiBatch->setMaterial(defaultSUIMaterial);
    uiBatch->state.position.z = 0.0;
    uiBatch->state.forceUpdate();

    FastUI_context ui(uiBatch, font, scene2D, defaultFontMaterial);
    FastUI_valueMenu menu(ui, {});

    BenchTimer physicsTimer("Physics Timer");

    menu->state.setPosition(vec3(-0.9, 0.5, 0)).scaleScalar(0.95);
    physicsTimer.setMenu(menu);
    // globals.appTime.setMenuConst(menu);
    // globals.cpuTime.setMenu(menu);
    // globals.gpuTime.setMenu(menu);
    // globals.fpsLimiter.setMenu(menu);
    

    menu.batch();
    scene2D.updateAllObjects();
    uiBatch->batch();

    /* Main Loop */
    while (state != quit)
    {
        mainloopStartRoutine();

        for (GLFWKeyInfo input; inputs.pull(input); userInput(input))
            ;

        if (move_forward)
        {
            // move camera forward
            vec3 forwarddir = globals.currentCamera->getDirection();

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + forwarddir * 0.1f);
        }

        if (move_backward)
        {
            // move camera backward
            vec3 backward = -globals.currentCamera->getDirection();

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + backward * 0.1f);
        }

        if (move_left)
        {
            // move camera left
            vec3 forwarddir = globals.currentCamera->getDirection();
            vec3 left = cross(vec3(0, 1, 0), forwarddir);

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + left * 0.1f);
        }

        if (move_right)
        {
            // move camera right
            vec3 forwarddir = globals.currentCamera->getDirection();
            vec3 right = cross(forwarddir, vec3(0, 1, 0));

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + right * 0.1f);
        }

        if (move_down)
        {
            // move camera down
            vec3 forwarddir = globals.currentCamera->getDirection();
            vec3 down = cross(forwarddir, vec3(0, 0, 1));

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + down * 0.1f);
        }

        if (move_up)
        {
            // move camera up
            vec3 forwarddir = globals.currentCamera->getDirection();
            vec3 up = cross(vec3(0, 0, 1), forwarddir);

            globals.currentCamera->setPosition(globals.currentCamera->getPosition() + up * 0.1f);
        }

        float delta = min(globals.simulationTime.getDelta(), 0.05f);
        if (globals.windowHasFocus() && delta > 0.00001f && !physicsPaused)
        {
            physicsTimer.start();

            physicsEngine.tick(delta);
            CubeGameObject.update();
            for (GameObject& sphereGameObject : sphereGameObjects) {
                sphereGameObject.update();
            }

            // std::cout << CubeGameObject.getBody()->getPosition().y << "\n";

            physicsTimer.end();
        }

        if (physicsPaused && step)
        {
            physicsEngine.tick(delta);
            //FloorGameObject.update();
            CubeGameObject.update();
            for (GameObject& sphereGameObject : sphereGameObjects) {
                sphereGameObject.update();
            }
            step = false;
        }

        menu.trackCursor();
        menu.updateText();

        mainloopPreRenderRoutine();

        /* UI & 2D Render */
        glEnable(GL_BLEND);
        glEnable(GL_FRAMEBUFFER_SRGB);

        scene2D.updateAllObjects();
        uiBatch->batch();
        screenBuffer2D.activate();
        uiBatch->draw();
        scene2D.draw();
        screenBuffer2D.deactivate();

        /* 3D Pre-Render */
        glDisable(GL_FRAMEBUFFER_SRGB);
        glDisable(GL_BLEND);
        glDepthFunc(GL_GREATER);
        glEnable(GL_DEPTH_TEST);

        scene.updateAllObjects();
        scene.generateShadowMaps();

        /* 3D Early Depth Testing */
        renderBuffer.activate();
        scene.depthOnlyDraw(*globals.currentCamera, true);
        glDepthFunc(GL_EQUAL);

        /* 3D Render */
        skybox->bindMap(0, 4);
        scene.genLightBuffer();
        scene.draw();
        renderBuffer.deactivate();

        /* Post Processing */
        renderBuffer.bindTextures();
        SSAO.render(*globals.currentCamera);
        Bloom.render(*globals.currentCamera);

        /* Final Screen Composition */
        glViewport(0, 0, globals.windowWidth(), globals.windowHeight());
        finalProcessingStage.activate();
        sun->shadowMap.bindTexture(0, 6);
        screenBuffer2D.bindTexture(0, 7);
        globals.drawFullscreenQuad();

        sun->shadowCamera.setPosition(globals.currentCamera->getPosition());
        // printm4(sun->shadowCamera.getProjectionViewMatrix());

        /* Main loop End */
        mainloopEndRoutine();
    }
}
