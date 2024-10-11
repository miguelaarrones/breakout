#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>

#include "Game.h"
#include "ResourceManager.h"

// GLFW callback functions
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT= 600;

Game Breakout(SCREEN_WIDTH, SCREEN_HEIGHT);

int main(int argc, char *argv[])
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef _APPLE_
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	glfwWindowHint(GLFW_RESIZABLE, false);

	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// GLAD: Load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialzie GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);

	// OpenGL configuration
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Initialize game
	Breakout.Init();

	// Delta time variables
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		// Calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		// Manage user input
		Breakout.ProcessInput(deltaTime);

		// Update game state
		Breakout.Update(deltaTime);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		Breakout.Render();

		glfwSwapBuffers(window);
	}

	// Delete all loaded resources using the resource manager
	ResourceManager::Clear();

	glfwTerminate();
	return 0;

}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// Make ssuire the viewport matches the new window dimensions
	// note: width and height will be significantly larger than specified on retina displays
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key >= 0 && key < 1024)
		if (action == GLFW_PRESS)
			Breakout.Keys[key] = true;
		else if (action == GLFW_RELEASE)
			Breakout.Keys[key] = false;
}
