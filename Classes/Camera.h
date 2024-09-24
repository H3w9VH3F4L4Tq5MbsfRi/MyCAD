#pragma once

#include<GLFW/glfw3.h>
#include<glm/glm.hpp>

class Camera
{
public:
	glm::vec3 Position;
	glm::vec3 Orientation;
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	bool firstClick = true;

	int GetWidth() { return width - guiWidth; }
    int GetHeight() { return height; }
    void SetWidth(int nWidth) { width = nWidth; }
    void SetHeight(int nHeight) { height = nHeight; }

	float speed = 0.01f;
	float sensitivity = 100.0f;
    const float speedStep = 0.01f;

	float FOV;
    float near;
    float far;

	int guiWidth;

	Camera(int width, int height, glm::vec3 position, float FOV, float near,
           float far, int guiWidth = 0);

	void PrepareMatrices(glm::mat4 &view, glm::mat4 &proj);
    void PrepareAnaglyphMatrices(float convergence, float eyeSeparation, glm::mat4 &projL, glm::mat4 &projR);
	void HandleInputs(GLFWwindow* window);

private:
    void KeyboardInputs(GLFWwindow *window);
	void MouseInputs(GLFWwindow *window);
    float GetAspectRatio();

    int width;
    int height;
};