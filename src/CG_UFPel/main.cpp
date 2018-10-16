#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>
#include <glm/gtx/spline.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void render(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> transform);
void processInput(GLFWwindow *window, vector<Model> objs, vector<int> *models, vector<glm::mat4> *transform, Shader shader);
void printState();
void createModel(const int obj, vector<int> *models, vector<glm::mat4> *transform);
void deleteModel(vector<int> *models, vector<glm::mat4> *transform);
void setDimension(GLFWwindow *window, int key);
void translate(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const char axis, const int sign);
void rotate(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const char axis, const int sign);
void scale(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const int sign);
void shear(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const int x, const int y);
glm::mat4 bigRotation(glm::mat4 mat, float deg, glm::vec3 rot, glm::vec3 transl);
void animation1(GLFWwindow *window, vector<int> *models, vector<glm::mat4> *transform, Shader shader);
void animation2(GLFWwindow *window, vector<int> *models, vector<glm::mat4> *transform, Shader shader);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int nModels = 0;
int position = 0;
int activeModel = 0;
int focus = 0;
glm::vec3 dim(1.0, 1.0, 1.0);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shader("resources/cg_ufpel.vs", "resources/cg_ufpel.fs");
    Model rock("resources/objects/rock/rock.obj");
    Model planet("resources/objects/planet/planet.obj");
    Model cyborg("resources/objects/cyborg/cyborg.obj");
    Model nanosuit("resources/objects/nanosuit/nanosuit.obj");
    vector<Model> objs;
    vector<int> models;
    vector<glm::mat4> transform;

    objs.push_back(rock);
    objs.push_back(planet);
    objs.push_back(cyborg);
    objs.push_back(nanosuit);
    // load models
    // -----------

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    createModel(0, &models, &transform);
    printState();

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, objs, &models, &transform, shader);

        render(window, shader, objs, models, transform);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void render(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> transform) {
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    // don't forget to enable shader before setting uniforms
    shader.use();

    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    for(int i = 0; i < nModels; ++i) {
        // render the loaded model
        shader.setMat4("model", transform[i]);
        objs[models[i]].Draw(shader);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, vector<Model> objs, vector<int> *models, vector<glm::mat4> *transform, Shader shader)
{
    float x = dim.x, y = dim.y, z = dim.z;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Model creation
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        createModel(0, models, transform);
        while(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        createModel(1, models, transform);
        while(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        createModel(2, models, transform);
        while(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        createModel(3, models, transform);
        while(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            glfwPollEvents();
    }

    // Delete active model
    if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        deleteModel(models, transform);
        while(glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS)
            glfwPollEvents();
    }

    // Select model
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        activeModel = (activeModel + 1) % nModels;
        printState();
        while(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        activeModel -= 1;
        if(activeModel < 0)

            activeModel = nModels - 1;
        printState();
        while(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            glfwPollEvents();
    }

    // Dimension selector
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        setDimension(window, GLFW_KEY_X);
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        setDimension(window, GLFW_KEY_Y);
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        setDimension(window, GLFW_KEY_Z);
    // Translation
    if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_6, 'x', 1);
    if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_4, 'x', -1);
    if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_8, 'y', 1);
    if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_2, 'y', -1);
    if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_7, 'z', 1);
    if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
        translate(window, shader, objs, *models, transform, GLFW_KEY_KP_9, 'z', -1);

    // Set rotation focus
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        focus = ((int) focus + 1) % 2;
        printState();
        while(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            glfwPollEvents();
    }
    // Rotation
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_I, 'x', -1);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_K, 'x', 1);
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_J, 'y', -1);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_L, 'y', 1);
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_U, 'z', 1);
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        rotate(window, shader, objs, *models, transform, GLFW_KEY_O, 'z', -1);

    // Scaling
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
        scale(window, shader, objs, *models, transform, GLFW_KEY_KP_SUBTRACT, -1);
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
        scale(window, shader, objs, *models, transform, GLFW_KEY_KP_ADD, 1);

    // Reflection
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        (*transform)[activeModel] = glm::scale((*transform)[activeModel], glm::vec3(-40.0f * x + 1.0f, -40.0f * y + 1.0f, -40.0f * z + 1.0f));
        while(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            glfwPollEvents();
    }

    // Shear
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_V, 'x', -1.0);
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_N, 'x', 1.0);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_G, 'y', -1.0);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_B, 'y', 1.0);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_F, 'z', -1.0);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        shear(window, shader, objs, *models, transform, GLFW_KEY_H, 'z', 1.0);

    // Projection
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        (*transform)[activeModel] = glm::proj3D((*transform)[activeModel], glm::vec3(1.0f*x, 1.0f*y, 1.0f*z));
        while(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            glfwPollEvents();
    }

    // Animations
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        animation1(window, models, transform, shader);
        while(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS) {
        animation2(window, models, transform, shader);
        while(glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
            glfwPollEvents();
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

void setDimension(GLFWwindow *window, int key) {
    float *dimension;
    if(key == GLFW_KEY_X)
        dimension = &dim.x;
    if(key == GLFW_KEY_Y)
        dimension = &dim.y;
    if(key == GLFW_KEY_Z)
        dimension = &dim.z;

    if(*dimension > 0)
        *dimension = 0.0;
    else
        *dimension = 1.0;

    printState();
    while(glfwGetKey(window, key) == GLFW_PRESS)
        glfwPollEvents();
}

void createModel(const int obj, vector<int> *models, vector<glm::mat4> *transform) {
    ++nModels;

    glm::mat4 mat;
    mat = glm::translate(mat, glm::vec3((float) position++, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    mat = glm::scale(mat, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down;
    transform->push_back(mat);

    models->push_back(obj);
    activeModel = nModels - 1;
    printState();
}

void deleteModel(vector<int> *models, vector<glm::mat4> *transform) {
    models->erase(models->begin() + activeModel);
    transform->erase(transform->begin() + activeModel);
    --nModels;

    if(activeModel == nModels)
        activeModel -= 1;
    printState();
}

void printState() {
    printf("\n ##############################################################################\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf( " #                              Active model: %d                               #\n", activeModel + 1);
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    if(!focus)
        printf(" #                           Rotation Focus: Axis                             #\n");
    else
        printf(" #                           Rotation Focus: Point                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    printf(  " #                                                                            #\n");
    if(dim.x > 0)
        printf(" #                          X: ON   ");
    else
        printf(" #                          X: OFF  ");
    if(dim.y > 0)
        printf("Y: ON   ");
    else
        printf("Y: OFF  ");
    if(dim.z > 0)
        printf("Z: ON                             #");
    else
        printf("Z: OFF                            #");
    printf("\n #                                                                            #");
    printf("\n #                                                                            #");
    printf("\n #                                                                            #");
    printf("\n #                                                                            #");
    printf("\n ##############################################################################\n\n");
}

void animation1(GLFWwindow *window, vector<int> *models, vector<glm::mat4> *transform, Shader shader) {
    int n = nModels;
    for(int i = 0; i < n; ++i) {
        deleteModel(models, transform);
    }
    vector<Model> elems;
    vector<glm::mat4> mats;

    Model planet("resources/objects/planet/planet.obj");
    Model rock("resources/objects/rock/rock.obj");
    Model dog("resources/objects/doggo/planet.obj");

    elems.push_back(planet);
    elems.push_back(rock);
    elems.push_back(rock);
    elems.push_back(rock);
    elems.push_back(rock);
    elems.push_back(rock);

    glm::mat4 matPlanet;
    glm::mat4 matRock1;
    glm::mat4 matRock2;
    glm::mat4 matRock3;
    glm::mat4 matRock4;
    glm::mat4 matRock5;

    matPlanet = glm::scale(matPlanet, glm::vec3(0.1f, 0.1f, 0.1f));
    matRock1 = glm::translate(matPlanet, glm::vec3(-7.5f, 0.0f, 0.0f));
    matRock1 = glm::scale(matRock1, glm::vec3(0.5f, 0.5f, 0.5f));
    matRock1 = glm::rotate(matRock1, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matRock1 = glm::rotate(matRock1, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    matRock2 = glm::translate(matPlanet, glm::vec3(0.0f, 7.5f, 0.0f));
    matRock2 = glm::scale(matRock2, glm::vec3(0.5f, 0.5f, 0.5f));
    matRock2 = glm::rotate(matRock2, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    matRock3 = glm::translate(matPlanet, glm::vec3(0.0f, -7.5f, 0.0f));
    matRock3 = glm::scale(matRock3, glm::vec3(0.5f, 0.5f, 0.5f));
    matRock3 = glm::rotate(matRock3, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    matPlanet = glm::translate(matPlanet, glm::vec3(-0.5f, 0.0f, 0.0f));
    matPlanet = glm::rotate(matPlanet, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    matRock4 = glm::translate(matPlanet, glm::vec3(0.0f, 0.0f, 7.5f));
    matRock4 = glm::scale(matRock4, glm::vec3(0.5f, 0.5f, 0.5f));
    matRock4 = glm::rotate(matRock4, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    matRock4 = glm::rotate(matRock4, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    matRock5 = glm::translate(matPlanet, glm::vec3(0.0f, 0.0f, -7.5f));
    matRock5 = glm::scale(matRock5, glm::vec3(0.5f, 0.5f, 0.5f));
    matRock5 = glm::rotate(matRock5, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

    mats.push_back(matPlanet);
    mats.push_back(matRock1);
    mats.push_back(matRock2);
    mats.push_back(matRock3);
    mats.push_back(matRock4);
    mats.push_back(matRock5);

    float timer = 0.0;
    glm::mat4 view = camera.GetViewMatrix();
    int change = 0;
    while(timer < 10.0) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timer += deltaTime;

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view;

        mats[0] = glm::rotate(mats[0], glm::radians(deltaTime*180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        for(int i = 1; i < 6; ++i) {
            if(timer < 4.5) {
                mats[i] = bigRotation(mats[i], glm::radians(deltaTime*180.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(-deltaTime*90, 0.0f, 0.0f));
            }
            if(timer > 3.5) {
                mats[i] = glm::translate(mats[i], glm::vec3(deltaTime*300.0, deltaTime*300.0, deltaTime*300.0));
            }
        }
        if(timer > 3.25 && timer < 4.0) {
            mats[0] = glm::scale(mats[0], glm::vec3(1.0f - deltaTime*6, 1.0f - deltaTime*6, 1.0f - deltaTime*6));
        }
        if(timer > 4.5 && change == 0) {
            change = 1;
            elems[0] = dog;
            // mats[0] = glm::mat4();
            mats[0] = glm::scale(matPlanet, glm::vec3(0.01, 0.01, 0.01));
            mats[0] = glm::rotate(mats[0], glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            mats[0] = glm::rotate(mats[0], glm::radians(15.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        }
        if(timer > 5.5 && timer < 6.0) {
            mats[0] = glm::scale(mats[0], glm::vec3(1.0f + deltaTime*11, 1.0f + deltaTime*11, 1.0f + deltaTime*11));
        }
        // don't forget to enable shader before setting uniforms
        shader.use();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        for(int i = 0; i < 6; ++i) {

            // render the loaded model
            shader.setMat4("model", mats[i]);
            elems[i].Draw(shader);
        }
        glfwSwapBuffers(window);
    }
}

void animation2(GLFWwindow *window, vector<int> *models, vector<glm::mat4> *transform, Shader shader) {
    int n = nModels;
    for(int i = 0; i < n; ++i) {
        deleteModel(models, transform);
    }
    vector<Model> elems;
    vector<glm::mat4> mats;

    Model planet("resources/objects/planet/planet.obj");
    elems.push_back(planet);

    glm::mat4 mat;

    glm::vec3 cp1 = glm::vec3(-1.5, 0.0, 0.0);
    glm::vec3 cp2 = glm::vec3(-1.0, 0.5, -0.5);
    glm::vec3 cp3 = glm::vec3(1.0, -0.5, -0.5);
    glm::vec3 cp4 = glm::vec3(1.5, -0.5, 0.0);

    mats.push_back(mat);
    mats.push_back(mat);
    mats.push_back(mat);
    mats.push_back(mat);
    mats.push_back(mat);

    mats[1] = glm::translate(mats[1], cp1);
    mats[1] = glm::scale(mats[1], glm::vec3(0.01, 0.01, 0.01));
    mats[2] = glm::translate(mats[2], cp2);
    mats[2] = glm::scale(mats[2], glm::vec3(0.01, 0.01, 0.01));
    mats[3] = glm::translate(mats[3], cp3);
    mats[3] = glm::scale(mats[3], glm::vec3(0.01, 0.01, 0.01));
    mats[4] = glm::translate(mats[4], cp4);
    mats[4] = glm::scale(mats[4], glm::vec3(0.01, 0.01, 0.01));

    float timer = 0.0;
    float t;
    glm::vec3 cp;
    mats[0] = glm::translate(mats[0], glm::vec3(-20.0, 0.0, 0.0));
    while(timer < 5.0) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timer += deltaTime;

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        cp = glm::cubic<glm::vec3>(cp4, cp3, cp2, cp1, timer/5);

        glm::vec3 old = glm::vec3(mats[0][3][0], mats[0][3][1], mats[0][3][2]);
        glm::vec3 trans = cp - old;
        mats[0] = glm::translate(mats[0], trans);
        mat = glm::scale(mats[0], glm::vec3(0.04f, 0.04f, 0.04f));
        for(int i = 1; i < 5; ++i) {
            // don't forget to enable shader before setting uniforms
            shader.use();

            shader.setMat4("projection", projection);
            shader.setMat4("view", view);
            // render the loaded model
            shader.setMat4("model", mats[i]);
            elems[0].Draw(shader);
        }
        // don't forget to enable shader before setting uniforms
        shader.use();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        // render the loaded model
        shader.setMat4("model", mat);
        elems[0].Draw(shader);
        glfwSwapBuffers(window);
    }
}

void translate(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const char axis, const int sign) {
    float currentFrame, x = 0.0, y = 0.0, z = 0.0;
    if(axis == 'x')
        x = 1.0 * sign;
    if(axis == 'y')
        y = 1.0 * sign;
    if(axis == 'z')
        z = 1.0 * sign;

    while(glfwGetKey(window, key) == GLFW_PRESS) {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        (*transform)[activeModel] = glm::translate((*transform)[activeModel], glm::vec3(deltaTime*2.0*x, deltaTime*2.0*y, deltaTime*2.0*z));
        render(window, shader, objs, models, *transform);
    }
}

void rotate(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const char axis, const int sign) {
    float currentFrame, x = 0.0, y = 0.0, z = 0.0;
    if(axis == 'x')
        x = 1.0 * sign;
    if(axis == 'y')
        y = 1.0 * sign;
    if(axis == 'z')
        z = 1.0 * sign;

    while(glfwGetKey(window, key) == GLFW_PRESS) {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if(focus)
            (*transform)[activeModel] = bigRotation((*transform)[activeModel], glm::radians(deltaTime*120.0f), glm::vec3(x, y, z), glm::vec3((-deltaTime*15*y) + (-deltaTime*15*z), -deltaTime*15*x, 0.0f));
        else
            (*transform)[activeModel] = glm::rotate((*transform)[activeModel], glm::radians(deltaTime*120.0f), glm::vec3(x, y, z));
        render(window, shader, objs, models, *transform);
    }
}

void scale(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const int sign) {
    float currentFrame, param, x = dim.x, y = dim.y, z = dim.z;

    while(glfwGetKey(window, key) == GLFW_PRESS) {
        currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;

        param = deltaTime*sign;

        (*transform)[activeModel] = glm::scale((*transform)[activeModel], glm::vec3(1.0f + (param*x), 1.0f + (param*y), 1.0f + (param*z)));
        render(window, shader, objs, models, *transform);
    }
}

void shear(GLFWwindow *window, Shader shader, vector<Model> objs, vector<int> models, vector<glm::mat4> *transform, const int key, const int axis, const int sign) {
    float currentFrame, param;

    while(glfwGetKey(window, key) == GLFW_PRESS) {
        currentFrame = glfwGetTime();
        deltaTime = (currentFrame - lastFrame);
        lastFrame = currentFrame;

        param = deltaTime*sign;
        if(axis == 'x')
            (*transform)[activeModel] = glm::shearX3D((*transform)[activeModel], param, param);
        if(axis == 'y')
            (*transform)[activeModel] = glm::shearY3D((*transform)[activeModel], param, param);
        if(axis == 'z')
            (*transform)[activeModel] = glm::shearZ3D((*transform)[activeModel], param, param);
        render(window, shader, objs, models, *transform);
    }
}

glm::mat4 bigRotation(glm::mat4 mat, float deg, glm::vec3 rot, glm::vec3 transl) {
    mat = glm::rotate(mat, deg, glm::vec3(rot.x, rot.y, rot.z));
    mat = glm::translate(mat, glm::vec3(transl.x, transl.y, transl.z));
    mat = glm::rotate(mat, deg, glm::vec3(rot.x, rot.y, rot.z));

    return mat;
}