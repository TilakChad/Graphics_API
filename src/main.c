#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../includes/renderer.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
	{
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void frame_change_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to load GLFW API. Exiting ... \n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1200, 800, "Graphics", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "Failed to create new window. Exiting .. . \n");
		return -2;
	}

	// So context must be created before loading the glad
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, frame_change_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to load Glad Loader. Exiting ...");
		return -3;
	}

	// Let's use modern opengl 
	Renderer render_engine;
	frameBuffer frame_buffer;
	initialize_renderer(&render_engine, &frame_buffer, 1200, 800);
	GLuint color_loc = glGetUniformLocation(render_engine.shader_program, "color_code");
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(render_engine.shader_program);
		
		glUniform1i(color_loc, 0);
		glBindVertexArray(render_engine.origin_vertex_array);
		glDrawArrays(GL_LINES, 0,render_engine.vertices_count[0]/2);

		glUniform1i(color_loc, 2);
		glBindVertexArray(render_engine.plot.plot_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		glUniform1i(color_loc, 1);
		glBindVertexArray(render_engine.scr_vertex_array);
		glDrawArrays(GL_TRIANGLES, 0, render_engine.vertices_count[1]);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

