#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shader.h"
#include "Sphere.h"
#include "Plane.h"
#include "Material.h"
#include "Light.h"
#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
float fov = 90.0f;
float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;

Camera camera(glm::vec3(1.5, 0, 30.0f));
bool MovementTrigger = false;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::mat4 trans = glm::mat4(1.0f);

int main() {
    #pragma region OpenGL Initializaion
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Path Tracer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseMovementCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    #pragma endregion

    Shader tracerShader("VertexShader.vs", "FragmentShader.fs");
    Shader viewShader("VertexShader.vs", "ViewFragmentShader.fs");

    float ff = tan(glm::radians(fov * 0.5f));
    float vertices[] = {
        -aspectRatio*ff,  ff, -1.0,
         aspectRatio*ff,  ff, -1.0,
         aspectRatio*ff, -ff, -1.0,
        -aspectRatio*ff, -ff, -1.0
    };
    int indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glm::mat4 proj = glm::mat4(1.0f);
    proj = glm::perspective(glm::radians(fov * 0.5f), aspectRatio, 0.1f, 100.0f);
    tracerShader.use();
    tracerShader.setMat4("proj", proj);
    tracerShader.setFloat("view_pixel_width", (float)(2.0f * aspectRatio * ff / SCR_WIDTH));
    tracerShader.setFloat("view_pixel_height", (float)(2.0f * ff / SCR_HEIGHT));

    unsigned int VAO,VBO,EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    #pragma region Spheres
    const int sphereCount = 7;
    Sphere sphereList[sphereCount] = {
            Sphere(glm::vec3(0.0f, 0.0f, 0.0f), 2.0f, Material(false, true, glm::vec3(1.0f, 1.0f, 1.0f))),      //metallic1
            Sphere(glm::vec3(6.0f, -0.5f, 4.0f), 1.5f, Material(false, true, glm::vec3(1.0f, 0.7f, 0.4f))),     //metallic2
            Sphere(glm::vec3(1.0f, -1.5f, 5.0f), 0.5f, Material(true, false, glm::vec3(1.0f, 0.0f, 0.0f))),     //red
            Sphere(glm::vec3(-2.0f, -1.0f, 6.0f), 1.0f, Material(true, false, glm::vec3(0.9f, 0.5f, 0.9f))),    //purple pink
            Sphere(glm::vec3(3.0f, -1.0f, 4.0f), 1.0f, Material(true, false, glm::vec3(0.0f, 1.0f, 0.0f))),     //green
            Sphere(glm::vec3(4.5f, -1.5f, 8.0f), 0.5f, Material(true, false, glm::vec3(1.0f, 0.6f, 0.5f))),     //mellow pink
            Sphere(glm::vec3(-3.0f, -1.5f, 8.0f), 0.5f, Material(true, false, glm::vec3(1.0f, 1.0f, 0.0f))),    //yellow
    };
    tracerShader.use();
    for (int i = 0; i < sphereCount; i++) {
        tracerShader.setVec3("spheres[" + std::to_string(i) + "].center", sphereList[i].center);
        tracerShader.setFloat("spheres[" + std::to_string(i) + "].radius",sphereList[i].radius);
        tracerShader.setBool("spheres[" + std::to_string(i) + "].mtl.diffuse", sphereList[i].mtl.diffuse);
        tracerShader.setBool("spheres[" + std::to_string(i) + "].mtl.metallic", sphereList[i].mtl.metallic);
        tracerShader.setVec3("spheres[" + std::to_string(i) + "].mtl.attenuation", sphereList[i].mtl.attenuation);
    }
    #pragma endregion

    #pragma region Planes  
    const int planeCount = 5;
    Plane planes[planeCount] = {
        Plane(glm::vec3(0,1.0f,0), glm::vec3(0,-2.0f,0), 100.0f, Material(true, false, glm::vec3(0.5f, 0.5f, 0.5f))),  //ground
        Plane(glm::vec3(0.7071067f,0.0f,0.7071067f), glm::vec3(-5.0f,0,0), 5.0f, Material(false, true, glm::vec3(1.0f, 1.0f, 1.0f))), //mirror1
        Plane(glm::vec3(0.7071067f,0.0f,0.7071067f), glm::vec3(-5.0f,0,0), 5.5f, Material(true, false, glm::vec3(0.4f, 0.3f, 0.9f))), //background
        Plane(glm::vec3(-0.7071067f,0.0f,0.7071067f), glm::vec3(5,0,-8.0f), 10.0f, Material(false, true, glm::vec3(1.0f, 1.0f, 1.0f))), //mirror
        Plane(glm::vec3(-0.7071067f,0.0f,0.7071067f), glm::vec3(5,0,-8.0f), 10.5f, Material(true, false, glm::vec3(0.9f, 1.0f, 0.8f))), //background
    };
    tracerShader.use();
    for (int i = 0; i < planeCount; i++) {
        tracerShader.setVec3("planes[" + std::to_string(i) + "].normal", planes[i].normal);
        tracerShader.setVec3("planes[" + std::to_string(i) + "].position", planes[i].position);
        tracerShader.setFloat("planes[" + std::to_string(i) + "].lenght", planes[i].lenght);
        tracerShader.setBool("planes[" + std::to_string(i) + "].mtl.diffuse", planes[i].mtl.diffuse);
        tracerShader.setBool("planes[" + std::to_string(i) + "].mtl.metallic", planes[i].mtl.metallic);
        tracerShader.setVec3("planes[" + std::to_string(i) + "].mtl.attenuation", planes[i].mtl.attenuation);
    }
    #pragma endregion

    #pragma region light sources  
    const int lightCount = 1;
    Light lights[lightCount] = {
        Light(glm::vec3(0.0f, 10.0f, 15.0f), glm::vec3(1.0f,1.0f,1.0f))
    };
    tracerShader.use();
    for (int i = 0; i < lightCount; i++) {
        tracerShader.setVec3("lights[" + std::to_string(i) + "].position", lights[i].position);
        tracerShader.setVec3("lights[" + std::to_string(i) + "].intensity", lights[i].intensity);
    }
    #pragma endregion

    #pragma region Render to Buffer
    GLint originalFrameBuffer;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
    //CREATE FRAME BUFFER
    float textureWidth = SCR_WIDTH;
    float textureHeight = SCR_HEIGHT;
    GLuint frameBuffer = 0;
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    //CREATE TEXTURE
    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //CREATE DEPTH BUFFER
    GLuint depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
    //CONFIGURE FRAME BUFFER
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
    GLenum drawbuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawbuffers);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }
    #pragma endregion

    unsigned int loopCount = 0;
    viewShader.use();
    viewShader.setMat4("proj", proj);
    viewShader.setUInt("width", SCR_WIDTH);
    viewShader.setUInt("height", SCR_HEIGHT);

    tracerShader.use();
    tracerShader.setUInt("width", SCR_WIDTH);
    tracerShader.setUInt("height", SCR_HEIGHT);
    //******************

    glm::mat4 view = glm::mat4(1.0);
    glm::vec2 randomVector_primary;
    glm::vec2 randomVector_scatter;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        loopCount++;

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //first pass
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, textureWidth, textureHeight);
        glClear(GL_DEPTH_BUFFER_BIT);

        tracerShader.use();
        if (MovementTrigger) {
            loopCount = 1;    
            tracerShader.setBool("cameraIsMoving", true);
            MovementTrigger = false;
        }
        else{
            tracerShader.setBool("cameraIsMoving", false);
        }
        
        view = camera.GetViewMatrix();
        tracerShader.setMat4("c2w", glm::inverse(view));
        tracerShader.setVec3("cameraPos", camera.getCameraPosition());
        randomVector_primary = glm::vec2(rand() / (RAND_MAX + 1.0), rand() / (RAND_MAX + 1.0));
        randomVector_scatter = glm::vec2(rand() / (RAND_MAX + 1.0), rand() / (RAND_MAX + 1.0));
        tracerShader.setVec2("randomVector", glm::vec2(rand() / (RAND_MAX + 1.0), rand() / (2 * (RAND_MAX + 1.0))));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);


        //second pass
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFrameBuffer);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        viewShader.use();
        viewShader.setMat4("c2w", glm::inverse(view));
        viewShader.setUInt("count", loopCount);


        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderedTexture);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteFramebuffers(1, &frameBuffer);

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.move(FORWARD, deltaTime);
        MovementTrigger = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move(BACKWARD, deltaTime);
        MovementTrigger = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(LEFT, deltaTime);
        MovementTrigger = true;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(RIGHT, deltaTime);
        MovementTrigger = true;
    }
    
}

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    camera.rotate(xpos, ypos);
    MovementTrigger = true;
}

