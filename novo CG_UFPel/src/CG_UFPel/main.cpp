#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform2.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, glm::mat4 *model, vector<Model> *models, vector<glm::mat4> *transform, Shader shader);
void printState();
void createModel(const char *obj, vector<Model> *models, vector<glm::mat4> *transform);
void deleteModel(vector<Model> *models, vector<glm::mat4> *transform);
void setDimension(GLFWwindow *window, int key);
glm::mat4 bigRotation(glm::mat4 mat, float deg, glm::vec3 rot, glm::vec3 transl);
void animation1(GLFWwindow *window, glm::mat4 *model, vector<Model> *models, vector<glm::mat4> *transform, Shader shaders);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
int nModels = 0;
int position = 0;
int activeModel = 0;
int focus = 0;
glm::vec3 dim(0.05, 0.05, 0.05);

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
    Shader shader(FileSystem::getPath("resources/cg_ufpel.vs").c_str(), FileSystem::getPath("resources/cg_ufpel.fs").c_str());
    vector<Model> models;
    vector<glm::mat4> transform;
    // load models
    // -----------

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------

    createModel("rock/rock.obj", &models, &transform);
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
        processInput(window, &transform[activeModel], &models, &transform, shader);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        for(int i = 0; i < nModels; ++i) {
            // don't forget to enable shader before setting uniforms
            shader.use();

            shader.setMat4("projection", projection);
            shader.setMat4("view", view);

            // render the loaded model
            shader.setMat4("model", transform[i]);
            models[i].Draw(shader);
        }


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, glm::mat4 *model, vector<Model> *models, vector<glm::mat4> *transform, Shader shader)
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
        createModel("rock/rock.obj", models, transform);
        while(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        createModel("planet/planet.obj", models, transform);
        while(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        createModel("cyborg/cyborg.obj", models, transform);
        while(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            glfwPollEvents();
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        createModel("nanosuit/nanosuit.obj", models, transform);
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
        *model = glm::translate(*model, glm::vec3(0.05f, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
        *model = glm::translate(*model, glm::vec3(-0.05f, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
        *model = glm::translate(*model, glm::vec3(0.0f, 0.05f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
        *model = glm::translate(*model, glm::vec3(0.0f, -0.05f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
        *model = glm::translate(*model, glm::vec3(0.0f, 0.0f, 0.05f));
    if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
        *model = glm::translate(*model, glm::vec3(0.0f, 0.0f, -0.05f));

    // Set rotation focus
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        focus = ((int) focus + 1) % 2;
        printState();
        while(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            glfwPollEvents();
        }
    }
    // Rotation
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(-0.01f, 0.0f, 0.0f), glm::vec3(0.0f, 0.5f, 0.0f));
        else
            *model = glm::rotate(*model, (float)0.05, glm::vec3(-0.01f, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(0.01f, 0.0f, 0.0f), glm::vec3(0.0f, -0.5f, 0.0f));
        else
            *model = glm::rotate(*model, (float)0.05, glm::vec3(0.01f, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(0.0f, -0.01f, 0.0f), glm::vec3(0.5f, 0.0f, 0.0f));
        else
            *model = glm::rotate(*model, (float)0.05, glm::vec3(0.0f, -0.01f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(-0.5f, 0.0f, 0.0f));
        else
            *model = glm::rotate(*model, (float)0.05, glm::vec3(0.0f, 0.01f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(0.0f, 0.0f, -0.01f), glm::vec3(0.5f, 0.0f, 0.0f));
        else
           *model = glm::rotate(*model, (float)0.05, glm::vec3(0.0f, 0.0f, 0.01f));
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if(focus)
            *model = bigRotation(*model, 0.05f, glm::vec3(0.0f, 0.0f, 0.01f), glm::vec3(-0.5f, 0.0f, 0.0f));
        else
           *model = glm::rotate(*model, (float)0.05, glm::vec3(0.0f, 0.0f, -0.01f));
    }

    // Scaling
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS)
        *model = glm::scale(*model, glm::vec3(1.0f - x, 1.0f - y, 1.0f - z));
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS)
        *model = glm::scale(*model, glm::vec3(1.0f + x, 1.0f + y, 1.0f + z));

    // Reflection
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        *model = glm::scale(*model, glm::vec3(-40.0f * x + 1.0f, -40.0f * y + 1.0f, -40.0f * z + 1.0f));
        while(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            glfwPollEvents();
        }
    }

    // Shear
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
        *model = glm::shearX3D(*model, -0.01f, -0.01f);
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        *model = glm::shearX3D(*model, 0.01f, 0.01f);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        *model = glm::shearY3D(*model, -0.01f, -0.01f);
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
        *model = glm::shearY3D(*model, 0.01f, 0.01f);
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        *model = glm::shearZ3D(*model, -0.01f, -0.01f);
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        *model = glm::shearZ3D(*model, 0.01f, 0.01f);

    // Animations
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
        animation1(window, model, models, transform, shader);
        while(glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS) {
            glfwPollEvents();
        }
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
        *dimension = 0.05;

    printState();
    while(glfwGetKey(window, key) == GLFW_PRESS)
        glfwPollEvents();
}

void createModel(const char *obj, vector<Model> *models, vector<glm::mat4> *transform) {
    ++nModels;

    glm::mat4 mat;
    mat = glm::translate(mat, glm::vec3((float) position++, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
    mat = glm::scale(mat, glm::vec3(0.1f, 0.1f, 0.1f));	// it's a bit too big for our scene, so scale it down;
    transform->push_back(mat);

    char name[40] = "resources/objects/";
    strcat(name, obj);
    Model model(FileSystem::getPath(name));
    models->push_back(model);
    activeModel = nModels - 1;
    printState();
}

void deleteModel(vector<Model> *models, vector<glm::mat4> *transform) {
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

void animation1(GLFWwindow *window, glm::mat4 *model, vector<Model> *models, vector<glm::mat4> *transform, Shader shader) {
    int n = nModels;
    vector<Model> *tempModels = models;
    vector<glm::mat4> *tempTransform = transform;
    for(int i = 0; i < n; ++i) {
        deleteModel(models, transform);
    }
    vector<Model> elems;
    vector<glm::mat4> mats;

    Model planet(FileSystem::getPath("resources/objects/planet/planet.obj"));
    Model rock1(FileSystem::getPath("resources/objects/rock/rock.obj"));
    Model rock2(FileSystem::getPath("resources/objects/rock/rock.obj"));
    Model rock3(FileSystem::getPath("resources/objects/rock/rock.obj"));
    Model rock4(FileSystem::getPath("resources/objects/rock/rock.obj"));
    Model rock5(FileSystem::getPath("resources/objects/rock/rock.obj"));
    elems.push_back(planet);
    elems.push_back(rock1);
    elems.push_back(rock2);
    elems.push_back(rock3);
    elems.push_back(rock4);
    elems.push_back(rock5);

    glm::mat4 matPlanet;
    glm::mat4 matRock1;
    glm::mat4 matRock2;
    glm::mat4 matRock3;
    glm::mat4 matRock4;
    glm::mat4 matRock5;

    matPlanet = glm::scale(matPlanet, glm::vec3(0.1f, 0.1f, 0.1f));
    matPlanet = glm::rotate(matPlanet, (float) 0.5, glm::vec3(0.0f, 0.0f, -0.01f));
    matRock1 = glm::translate(matRock1, glm::vec3(0.0f, 0.0f, -0.5f));
    matRock1 = glm::scale(matRock1, glm::vec3(0.07f, 0.07f, 0.07f));
    matRock2 = glm::translate(matRock2, glm::vec3(0.0f, 0.1f, 0.5f));
    matRock2 = glm::scale(matRock2, glm::vec3(0.07f, 0.07f, 0.07f));
    matRock3 = glm::translate(matRock3, glm::vec3(0.0f, 0.7f, 0.0f));
    matRock3 = glm::scale(matRock3, glm::vec3(0.07f, 0.07f, 0.07f));
    matRock4 = glm::translate(matRock4, glm::vec3(0.05f, 0.1f, 0.2f));
    matRock4 = bigRotation(matRock4, (float) 0.4, glm::vec3(0.0f, 0.0f, -0.01f), glm::vec3(0.1f, 0.0f, 0.0f));
    matRock4 = glm::scale(matRock4, glm::vec3(0.07f, 0.07f, 0.07f));
    matRock5 = glm::translate(matRock5, glm::vec3(0.05f, 0.1f, 0.2f));
    matRock5 = bigRotation(matRock5, (float) 0.4, glm::vec3(0.0f, 0.0f, 0.01f), glm::vec3(-0.1f, 0.0f, 0.0f));
    matRock5 = glm::scale(matRock5, glm::vec3(0.07f, 0.07f, 0.07f));
    matRock5 = bigRotation(matRock5, 0.5, glm::vec3(0.01f, 0.0f, 0.0f), glm::vec3(0.0f, -15.0f, 0.0f));

    mats.push_back(matPlanet);
    mats.push_back(matRock1);
    mats.push_back(matRock2);
    mats.push_back(matRock3);
    mats.push_back(matRock4);
    mats.push_back(matRock5);

    float start = glfwGetTime();
    float timer = 0.0;
    while(timer < 5.0) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        timer = currentFrame - start;
        printf("%f\n", timer);

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        mats[0] = glm::rotate(mats[0], (float) 0.05, glm::vec3(0.0f, 0.01f, 0.0f));
        mats[1] = bigRotation(mats[1], 0.055, glm::vec3(0.0f, 0.01f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        mats[2] = bigRotation(mats[2], 0.06, glm::vec3(-0.01f, 0.0f, 0.0f), glm::vec3(0.0f, 1.2f, 0.0f));
        mats[3] = bigRotation(mats[3], 0.055, glm::vec3(0.0f, 0.0f, 0.01f), glm::vec3(-1.0f, 0.0f, 0.0f));
        mats[4] = bigRotation(mats[4], 0.05, glm::vec3(-0.01f, 0.0f, 0.0f), glm::vec3(0.0f, 1.5f, 0.0f));
        mats[5] = bigRotation(mats[5], 0.05, glm::vec3(0.01f, 0.0f, 0.0f), glm::vec3(0.0f, -1.5f, 0.0f));

        for(int i = 0; i < 6; ++i) {
            // don't forget to enable shader before setting uniforms
            shader.use();

            shader.setMat4("projection", projection);
            shader.setMat4("view", view);

            // render the loaded model
            shader.setMat4("model", mats[i]);
            elems[i].Draw(shader);
        }
        glfwSwapBuffers(window);
    }
}

glm::mat4 bigRotation(glm::mat4 mat, float deg, glm::vec3 rot, glm::vec3 transl) {
    mat = glm::rotate(mat, deg, glm::vec3(rot.x, rot.y, rot.z));
    mat = glm::translate(mat, glm::vec3(transl.x, transl.y, transl.z));
    mat = glm::rotate(mat, deg, glm::vec3(rot.x, rot.y, rot.z));

    return mat;
}