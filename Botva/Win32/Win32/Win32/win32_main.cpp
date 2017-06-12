#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "CollectStatistics.h"

#include <iostream>

#pragma comment (lib, "glfw3.lib")

int main()
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1980, 1080, "win", nullptr/*glfwGetPrimaryMonitor()*/, nullptr);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	collect_statistics(
		[&window]() { glfwSwapBuffers(window); },
		1980,
		1080,
		"");

	glfwTerminate();
	return 0;
}