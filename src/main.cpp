#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <learnopengl/cubemap.h>
#include <learnopengl/vampire.h>

#include <iostream>
#include <memory>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 backpackPosition = glm::vec3(0.0f);
    float backpackScale = 1.0f;
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

struct Screen {
    unsigned width {SCR_WIDTH};
    unsigned height {SCR_HEIGHT};
} screen;

ProgramState *programState;
std::unique_ptr<Vampire> vampire;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(screen.width, screen.height, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");

    // load models
    // -----------
    Model terrain("resources/objects/grass/grass.obj");
    terrain.SetShaderTextureNamePrefix("material.");

    stbi_set_flip_vertically_on_load(false);
    Model barn("resources/objects/barn/barn.obj");
    barn.SetShaderTextureNamePrefix("material.");

    Model lantern("resources/objects/lantern/lantern.obj");
    lantern.SetShaderTextureNamePrefix("material.");

    Model pine("resources/objects/pine/pine.obj");
    pine.SetShaderTextureNamePrefix("material.");

    vampire = std::make_unique<Vampire>();
    stbi_set_flip_vertically_on_load(true);

    PointLight& pointLight = programState->pointLight;
    // pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    // position of lantern
    pointLight.position = glm::vec3(4.0f, 6.65f, -33.0f);
    /// TODO: revert to (0.1, 0.1, 0.1)
    pointLight.ambient = glm::vec3(1.0, 1.0, 1.0);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    // fixed height for FPS camera
    programState->camera.Position.y = 5.5f;

    CubeMap skybox {
        skyboxShader,
        {
            "resources/textures/skybox/px.png",
            "resources/textures/skybox/nx.png",
            "resources/textures/skybox/py.png",
            "resources/textures/skybox/ny.png",
            "resources/textures/skybox/pz.png",
            "resources/textures/skybox/nz.png"
        }
    };

    /// TODO: refactor
    std::vector<glm::vec3> pinePositions = {
        {-35.8, 1.5, -9.3},
        {-36.6, 1.5, -56.6},
        {37.0, 1.5, 52.2},
        {26.5, 1.5, -22.8},
        {18.1, 1.5, -0.6},
        {-39.4, 1.5, 27.3},
        {27.1, 1.5, -50.1},
        {30.0, 1.5, 48.9},
        {-27.6, 1.5, 60.3},
        {23.9, 1.5, 65.1},
        {-9.9, 1.5, -10.2},
        {9.7, 1.5, 48.3},
        {-11.9, 1.5, 47.8},
        {-19.1, 1.5, -48.0},
        {-24.1, 1.5, -26.8},
        {18.7, 1.5, -24.7},
        {-30.9, 1.5, -42.4},
        {-19.2, 1.5, 0.2},
        {28.8, 1.5, -30.8},
        {35.8, 1.5, 41.3},
        {15.3, 1.5, -66.8},
        {22.0, 1.5, 10.8},
        {13.1, 1.5, 68.5},
        {15.2, 1.5, 9.0},
        {-20.3, 1.5, -9.7},
        {36.6, 1.5, 6.1},
        {22.1, 1.5, -40.4},
        {15.9, 1.5, 23.9},
        {-30.2, 1.5, -5.8},
        {24.0, 1.5, -11.0},
        {-30.1, 1.5, 14.2},
        {-12.4, 1.5, 12.9},
        {39.0, 1.5, -32.4},
        {-21.9, 1.5, -56.6},
        {-32.7, 1.5, 20.0},
        {37.8, 1.5, 32.5},
        {37.6, 1.5, -7.9},
        {11.8, 1.5, 31.4},
        {10.1, 1.5, -6.5},
        {-18.8, 1.5, -67.9},
        {21.4, 1.5, -63.1},
        {-36.1, 1.5, 45.1},
        {-27.7, 1.5, 24.1},
        {27.4, 1.5, 33.6},
        {-35.9, 1.5, -48.4},
        {-23.6, 1.5, 15.1},
        {-27.2, 1.5, -49.8},
        {-29.4, 1.5, 51.5},
        {-39.0, 1.5, -41.6},
        {33.2, 1.5, -62.6},
    };

    std::vector<float> pineScales {
        2.7f, 3.4f, 5.5f, 5.7f, 2.6f, 2.6f, 3.6f, 5.9f, 3.9f, 3.2f,
        4.3f, 5.8f, 3.9f, 4.7f, 3.4f, 4.5f, 2.4f, 2.8f, 2.6f, 1.3f,
        3.1f, 4.6f, 1.7f, 2.6f, 1.9f, 5.5f, 5.0f, 1.9f, 4.8f, 1.5f,
        2.2f, 3.6f, 4.7f, 2.0f, 1.7f, 3.9f, 2.5f, 2.7f, 3.8f, 3.3f,
        3.8f, 3.8f, 1.2f, 5.0f, 3.5f, 2.9f, 1.3f, 5.6f, 5.3f, 5.9f
    };

    std::vector<glm::mat4> pineModels;
    for (auto i = 0u; i < pinePositions.size(); ++i) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f), pinePositions[i]);
        model = glm::scale(model, glm::vec3(pineScales[i]));
        pineModels.push_back(model);
    }

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);


        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        // pointLight.position = glm::vec3(4.0 * cos(currentFrame), 4.0f, 4.0 * sin(currentFrame));
        ourShader.uniform("pointLight.position", pointLight.position);
        ourShader.uniform("pointLight.ambient", pointLight.ambient);
        ourShader.uniform("pointLight.diffuse", pointLight.diffuse);
        ourShader.uniform("pointLight.specular", pointLight.specular);
        ourShader.uniform("pointLight.constant", pointLight.constant);
        ourShader.uniform("pointLight.linear", pointLight.linear);
        ourShader.uniform("pointLight.quadratic", pointLight.quadratic);
        ourShader.uniform("viewPosition", programState->camera.Position);
        ourShader.uniform("material.shininess", 32.0f);
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                static_cast<float>(screen.width) / static_cast<float>(screen.height),
                                                0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.uniform("projection", projection);
        ourShader.uniform("view", view);

        // render the loaded model
        // glm::mat4 model = glm::mat4(1.0f);
        // model = glm::translate(model,
        //                        programState->backpackPosition); // translate it down so it's at the center of the scene
        // model = glm::scale(model, glm::vec3(programState->backpackScale));    // it's a bit too big for our scene, so scale it down
        // ourShader.uniform("model", model);
        // ourModel.Draw(ourShader);

        glm::mat4 model = glm::mat4(1.0f);
        ourShader.uniform("model", model);
        terrain.Draw(ourShader);

        model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.9f, -40.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
        ourShader.uniform("model", model);
        barn.Draw(ourShader);

        model = glm::translate(glm::mat4(1.0f), glm::vec3(4.0f, 6.65f, -33.0f));
        model = glm::scale(model, glm::vec3(0.3f));
        ourShader.uniform("model", model);
        lantern.Draw(ourShader);

        for (const auto &modelMatrix : pineModels) {
            ourShader.uniform("model", modelMatrix);
            pine.Draw(ourShader);
        }

        vampire->draw(ourShader, currentFrame, deltaTime);

        skybox.draw(view, projection);

        if (programState->ImGuiEnabled)
            DrawImGui(programState);



        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    screen.width = width;
    screen.height = height;
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("Backpack position", (float*)&programState->backpackPosition);
        ImGui::DragFloat("Backpack scale", &programState->backpackScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        vampire->attack(programState->camera.Position, programState->camera.Front, glfwGetTime());
    }
}
