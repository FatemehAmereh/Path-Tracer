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

    Shader shader("VertexShader.vs", "FragmentShader.fs");

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
    shader.use();
    shader.setMat4("proj", proj);
    shader.setFloat("view_pixel_width", (float)(2.0f * aspectRatio * ff / SCR_WIDTH));
    shader.setFloat("view_pixel_height", (float)(2.0f * ff / SCR_HEIGHT));

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
    shader.use();
    for (int i = 0; i < sphereCount; i++) {
        shader.setVec3("spheres[" + std::to_string(i) + "].center", sphereList[i].center);
        shader.setFloat("spheres[" + std::to_string(i) + "].radius",sphereList[i].radius);
        shader.setBool("spheres[" + std::to_string(i) + "].mtl.diffuse", sphereList[i].mtl.diffuse);
        shader.setBool("spheres[" + std::to_string(i) + "].mtl.metallic", sphereList[i].mtl.metallic);
        shader.setVec3("spheres[" + std::to_string(i) + "].mtl.attenuation", sphereList[i].mtl.attenuation);
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
    shader.use();
    for (int i = 0; i < planeCount; i++) {
        shader.setVec3("planes[" + std::to_string(i) + "].normal", planes[i].normal);
        shader.setVec3("planes[" + std::to_string(i) + "].position", planes[i].position);
        shader.setFloat("planes[" + std::to_string(i) + "].lenght", planes[i].lenght);
        shader.setBool("planes[" + std::to_string(i) + "].mtl.diffuse", planes[i].mtl.diffuse);
        shader.setBool("planes[" + std::to_string(i) + "].mtl.metallic", planes[i].mtl.metallic);
        shader.setVec3("planes[" + std::to_string(i) + "].mtl.attenuation", planes[i].mtl.attenuation);
    }
    #pragma endregion

    #pragma region light sources  
    const int lightCount = 1;
    Light lights[lightCount] = {
        Light(glm::vec3(0.0f, 10.0f, 15.0f), glm::vec3(1.0f,1.0f,1.0f))
    };
    shader.use();
    for (int i = 0; i < lightCount; i++) {
        shader.setVec3("lights[" + std::to_string(i) + "].position", lights[i].position);
        shader.setVec3("lights[" + std::to_string(i) + "].intensity", lights[i].intensity);
    }
    #pragma endregion

    shader.setVec2("randomVector", glm::vec2(rand() / (RAND_MAX + 1.0), rand() / (2 * (RAND_MAX + 1.0))));

    glm::mat4 view = glm::mat4(1.0);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.6f, 0.5f, 0.5f, 1.0f);
        glClear( GL_DEPTH_BUFFER_BIT);

        shader.use();
        view = camera.GetViewMatrix();
        shader.setMat4("c2w", glm::inverse(view));
        shader.setVec3("cameraPos", camera.getCameraPosition());

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

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
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.move(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.move(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.move(RIGHT, deltaTime);
    }
}

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    camera.rotate(xpos, ypos);
}

