#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../includes/renderer.h"
#include <math.h>

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

float absolute(float x)
{
	if (x >= 0)
		return x;
	return -x;
}
void DDA_line(frameBuffer* frame_buffer,int x1, int y1, int x2, int y2)
{
	// plot line from (x1,y1) to (x2,y2) using DDA_line drawing algorithm
	float dx = x2 - x1;
	float dy = y2 - y1;

	int step;
	if (absolute(dx) > absolute(dy))
		step = absolute(dx);
	else
		step = absolute(dy);

	float x = x1, y = y1;
	dx = (float)dx / step; 
	dy = (float)dy / step;

	for (int i = 0; i <= step; ++i)
	{
		setPixel(frame_buffer, (Point) { (int) (x + 0.5f), (int)(y + 0.5f)});
		x += dx;
		y += dy;
		printf("%f %f\n",x,y);
	}
}

void sine_x(frameBuffer* frame_buffer,float x, float y1)
{
	// Use small increment .. sample at 1 pixel distance 
	// if we assume x and y as initial and final degree we can plot the graph centred at origin as


	float y;
	// It doesn't sample y correctly. Need some modficiation though.
	for (int i = x ; i <= y1 ;i++)
	{
		y = sin(i * 3.141592f / 180);
		setPixel(frame_buffer, (Point) { i, (int)(y*90 + 0.5) });
	}
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

	DDA_line(&frame_buffer, 1, 0, 7, 4);
	
	// sine_x(&frame_buffer, 0, 180);
	update_plot(&render_engine, &frame_buffer,(float)1200/800);


	GLuint color_loc = glGetUniformLocation(render_engine.shader_program, "color_code");
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(render_engine.shader_program);
		
		glUniform1i(color_loc, 2);
		glBindVertexArray(render_engine.plot.plot_VAO);
		glDrawArrays(GL_TRIANGLES, 0, render_engine.vertices_count[2]/2);
		
		glUniform1i(color_loc, 0);
		glBindVertexArray(render_engine.origin_vertex_array);
		glDrawArrays(GL_LINES, 0,render_engine.vertices_count[0]/2);

		
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

