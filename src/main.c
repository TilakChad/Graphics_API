#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../includes/renderer.h"
#include <math.h>
#include <time.h>

void DDA_line(frameBuffer* frame_buffer, int x1, int y1, int x2, int y2);
float absolute(float x);


// GLFW API provide a function with which we can set a single pointer and retrieve it wherever we desire
// But we may not to pass more pointers sometimes.
// So, wrapping the pointers within a struct might be the good choice. At least, that's what I am going to do.
// Its the struct whose pointer we are going to retrieve in the callback function.

typedef struct UserPtr {
	Renderer* render_ptr;
	frameBuffer* frame_ptr;
} UserPtr;

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
	UserPtr* ptr = (UserPtr*)glfwGetWindowUserPointer(window);
	assert(ptr != NULL);
	ptr->frame_ptr->aspect_ratio = (float)width / height;
	update_frame(ptr->render_ptr, ptr->frame_ptr);
}

// Callback function have no way of passing arguments directly to them. 
// Suppose, size of the window change, then the number of lines or size of pixel that appear on the screen change.
// In order to accomodate for that, we need to re initialize all the rendering 

void handle_key_press(GLFWwindow* window, Renderer* render_engine, frameBuffer* frame_buffer)
{
	bool should_update = false;
	float move_scale = 0.005f;;

	// lol .. we need aspect ratio here also
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		// Since our aspect ratio assumes y-axis to be the standard, we gotta divide x axis by aspect ratio 
		frame_buffer->origin_x += move_scale/frame_buffer->aspect_ratio ;
		should_update = true;
	}

	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		should_update = true;
		frame_buffer->origin_x -= move_scale / frame_buffer->aspect_ratio;
	}

	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		frame_buffer->origin_y -= move_scale;
		should_update = true;
	}

	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		frame_buffer->origin_y += move_scale;
		should_update = true;
	}

	if(should_update)
		update_frame(render_engine, frame_buffer);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Mouse wheel only provide offset in y direction .. 
	// Don't know how yoffset will scrolling produce and how it should be scaled for .. Gotta expermient some
	fprintf(stderr, "\nY-offset produced is : %lf.",yoffset);
	// so, rolling upward produce +1 offset and rolling toward you produce -1 offset
	// scaling operation is centered arond origin ..As a result, pixel position may change wrt to distance from origin.
	// so visible plotted point may not be visible in that frame. Scaling is uniform. 
	// 1 offset -> 0.02f
	float scale_sensitivity = 0.001f; // 5 unit rolls will produce a magnification by a scale of 0.1f (not absolute magnification)
	UserPtr* ptr = (UserPtr*)glfwGetWindowUserPointer(window);
	assert(ptr != NULL);
	// Lol .. scale got negative once 
	// assert(ptr->frame_ptr->scale_factor > 0.005f);
	fprintf(stderr, "\nScale sensitivity is : %lf.", ptr->frame_ptr->scale_factor);
	if ( (yoffset < -0.999f) && (ptr->frame_ptr->scale_factor <= 0.005f))
	{
		// Do nothing .. Hahahaha
		// Lol still crashing .. don't know the reason 
		fprintf(stderr, "\nAin't doing anything ..\n");
	}
	else
	{
		ptr->frame_ptr->scale_factor += yoffset * scale_sensitivity;
		update_frame(ptr->render_ptr, ptr->frame_ptr);
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
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to load Glad Loader. Exiting ...");
		return -3;
	}

	// Let's use modern opengl 
	Renderer render_engine;
	frameBuffer frame_buffer;

	// Set pointer for retrieving during callback function 
	UserPtr ptr;
	ptr.render_ptr = &render_engine;
	ptr.frame_ptr = &frame_buffer;
	glfwSetWindowUserPointer(window, &ptr);

	initialize_renderer(&render_engine, &frame_buffer, 1200, 800);

	DDA_line(&frame_buffer, 1, 0, 7, 4);
	
	// sine_x(&frame_buffer, 0, 180);
	update_plot(&render_engine, &frame_buffer);


	GLuint color_loc = glGetUniformLocation(render_engine.shader_program, "color_code");
	clock_t now = clock(), then = clock();
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(render_engine.shader_program);
		
		glUniform1i(color_loc, 2);
		glBindVertexArray(render_engine.plot.plot_VAO);
		glDrawArrays(GL_TRIANGLES, 0, render_engine.vertices_count[2]/2);
		
		glUniform1i(color_loc, 1);
		glBindVertexArray(render_engine.origin.origin_VAO);
		glDrawArrays(GL_TRIANGLES, 0,render_engine.vertices_count[1]/2);

		
		glUniform1i(color_loc, 0);
		glBindVertexArray(render_engine.grid.grid_VAO);
		glDrawArrays(GL_LINES, 0, render_engine.vertices_count[0]/2);

		now = clock();
		//fprintf(stderr, "\nFPS is -> %f.", (double)(now - then) / CLOCKS_PER_SEC);
		then = now;
		handle_key_press(window, &render_engine, &frame_buffer);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


float absolute(float x)
{
	if (x >= 0)
		return x;
	return -x;
}
void DDA_line(frameBuffer* frame_buffer, int x1, int y1, int x2, int y2)
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
		setPixel(frame_buffer, (Point) { (int)(x + 0.5f), (int)(y + 0.5f) });
		x += dx;
		y += dy;
		printf("%f %f\n", x, y);
	}
}

void sine_x(frameBuffer* frame_buffer, float x, float y1)
{
	// Use small increment .. sample at 1 pixel distance 
	// if we assume x and y as initial and final degree we can plot the graph centred at origin as


	float y;
	// It doesn't sample y correctly. Need some modficiation though.
	for (int i = x; i <= y1; i++)
	{
		y = sin(i * 3.141592f / 180);
		setPixel(frame_buffer, (Point) { i, (int)(y * 90 + 0.5) });
	}
}
