#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include "../includes/renderer.h"
#include "../includes/GraphicsAPI.h"
#include <math.h>
#include <time.h>


// It will take the current cursor position and move the frame if dragged
void mouse_panning(GLFWwindow* window, mouseState* cursor); // will get other information from userPoiner 

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
	Plotter* ptr = (Plotter*)glfwGetWindowUserPointer(window);
	assert(ptr != NULL);
	ptr->frame_info->aspect_ratio = (float)width / height;
	update_frame(ptr->render_engine, ptr->frame_info);
}

// Callback function have no way of passing arguments directly to them. 
// Suppose, size of the window change, then the number of lines or size of pixel that appear on the screen change.
// In order to accomodate for that, we need to re initialize all the rendering 

void handle_key_press(GLFWwindow* window, Renderer* render_engine, viewInfo* frame_info)
{
	bool should_update = false;
	float move_scale = 0.005f;;

	// lol .. we need aspect ratio here also
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		// Since our aspect ratio assumes y-axis to be the standard, we gotta divide x axis by aspect ratio 
		frame_info->origin_x += move_scale/frame_info->aspect_ratio ;
		should_update = true;
	}

	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		should_update = true;
		frame_info->origin_x -= move_scale / frame_info->aspect_ratio;
	}

	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		frame_info->origin_y -= move_scale;
		should_update = true;
	}

	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		frame_info->origin_y += move_scale;
		should_update = true;
	}

	if(should_update)
		update_frame(render_engine, frame_info);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Mouse wheel only provide offset in y direction .. 
	// Don't know how yoffset will scrolling produce and how it should be scaled for .. Gotta expermient some
	// so, rolling upward produce +1 offset and rolling toward you produce -1 offset
	// haha .. scroll fast and it will still produce an offset of +- 1 
	// 	   Scroll it super fast and voila.. you got +- 2 offset
	// Not interesting, right? .. But its enough to crash your already crashing program.  
	// scaling operation is centered arond origin ..As a result, pixel position may change wrt to distance from origin.
	// so visible plotted point may not be visible in that frame. Scaling is uniform. 
	// 1 offset -> 0.02f
	float scale_sensitivity = 0.001f; // 5 unit rolls will produce a magnification by a scale of 0.1f (not absolute magnification)
	Plotter* ptr = (Plotter*)glfwGetWindowUserPointer(window);
	assert(ptr != NULL);
	// Lol .. scale got negative once 
	// assert(ptr->frame_ptr->scale_factor > 0.005f);
#ifdef EN_LOG
	fprintf(stderr, "\nY-offset produced is -> %lf.",yoffset);
	fprintf(stderr, "\nCurrent scale factor is -> %lf.", ptr->frame_ptr->scale_factor);
#endif
	if ( (yoffset < -0.999f) && (ptr->frame_info->scale_factor <= 0.005f))
	{
		// Do nothing .. Hahahaha
		// Lol still crashing .. don't know the reason 
		fprintf(stderr, "\nAin't doing anything ..\n");
	}
	else
	{
		ptr->frame_info->scale_factor += yoffset * scale_sensitivity;
		update_frame(ptr->render_engine, ptr->frame_info);
	}
}

// Ok good to go 
// Now we will go for panning ..i.e moving the graph with mouse drag (:D Don't know if panning is the right word here though)
// To support panning we need to check if the mouse button is kept pressed and it is changing 
// Previous state of mouse and its position need to be stored and depending upon the difference in x and y coordinate of current 
// and previous state 
// hahaha .. let's put that into already passed struct frame_info .. Not relevant right .. but better than passing a new struct all over again



Plotter* createPlotter(int width, int hight, float scale)
{
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to load GLFW API. Exiting ... \n");
		return NULL;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(1200, 800, "Graphics", NULL, NULL);
	
	if (!window)
	{
		fprintf(stderr, "Failed to create new window. Exiting .. . \n");
		return NULL;
	}

	//// So context must be created before loading the glad
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, frame_change_callback);
	glfwSetScrollCallback(window, scroll_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		fprintf(stderr, "Failed to load Glad Loader. Exiting ...");
		return NULL;
	}

	//// Allocate plotter on the heap 
	// It need to be exlicitly allocated on the heap
	Plotter* plotter = malloc(sizeof(Plotter));

	////// Let's use modern opengl 
	Renderer* render_engine = malloc(sizeof(Renderer));
	viewInfo* frame_info = malloc(sizeof(viewInfo));

	// Set pointer for retrieving during callback function 

	glfwSetWindowUserPointer(window, plotter);

	initialize_renderer(render_engine, frame_info, 1200, 800);	
	////update_plot(render_engine, frame_info);

	mouseState* cursor_state = malloc(sizeof(mouseState));
	cursor_state->is_was_pressed = false;
	cursor_state->xpos = 0;
	cursor_state->ypos = 0;

	// Get all informatio back to the calling function
	plotter->window = window;
	plotter->frame_info = frame_info;
	plotter->render_engine = render_engine;
	plotter->cursor_state = cursor_state;
	return plotter;
}



void mouse_panning(GLFWwindow* window, mouseState* cursor)
{
	if (!cursor->is_was_pressed)
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
		{
			cursor->is_was_pressed = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			cursor->xpos = xpos;
			cursor->ypos = ypos;
		}
		return;
	}
	else
	{
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
		{
			cursor->is_was_pressed = false;
			return;
		}
		// This else is the case we are going to handle now below 
	}

	double x_pos, y_pos;
	glfwGetCursorPos(window, &x_pos, &y_pos);

	// It will perform the dragging operation now.. It's quite simple though.. Only origin need to be shifted
	// Every other thing will be handled auto 
	// Maybe I should try converting the obtained position in ndc and then update .. later

	double delta_xpos = x_pos - cursor->xpos;
	double delta_ypos = y_pos - cursor->ypos;

	// Ok .. to normalize this we need the information about current context ... but we don't have that one..
	// Let's hardcode it
	// Passing around might be a good idea but let's use glfw's function for now

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	// convert the obtained difference into ndc 

	float dx = 2 * delta_xpos / (width);
	float dy = 2 * delta_ypos / (height);

	// Maybe I should minimize use of userpointer . Don't know the performance penalty yet
	Plotter* ptr = (Plotter*)glfwGetWindowUserPointer(window);
	ptr->frame_info->origin_x += dx;
	ptr->frame_info->origin_y -= dy;
	update_frame(ptr->render_engine, ptr->frame_info);

	// Forgot to update the cursor_position 
	cursor->xpos = x_pos;
	cursor->ypos = y_pos;
}

void plot(Plotter* plotter)
{
	Renderer* render_engine = plotter->render_engine;
	GLFWwindow* window = plotter->window;
	viewInfo* frame_info = plotter->frame_info;
	mouseState* cursor_state = plotter->cursor_state;

	GLuint color_loc = glGetUniformLocation(render_engine->shader_program, "color_code");

	while (!glfwWindowShouldClose(window))
	{

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		// fprintf(stderr,"\nIt is : %d.",render_engine->shader_program);
		glUseProgram(render_engine->shader_program);

		if (render_engine->render_type != RENDER_NO_PIXELS)
		{
			glUniform1i(color_loc, 2);
			glBindVertexArray(render_engine->plot.plot_VAO);
			glDrawArrays(GL_TRIANGLES, 0, render_engine->vertices_count[2] / 2);
		}

		if ((render_engine->render_type != RENDER_NO_ORIGIN) && (render_engine->render_type != RENDER_NO_GRIDS_AND_ORIGINS))
		{
				glUniform1i(color_loc, 1);	
				glBindVertexArray(render_engine->origin.origin_VAO);
				glDrawArrays(GL_TRIANGLES, 0, render_engine->vertices_count[1] / 2);
		}

		if ((render_engine->render_type != RENDER_NO_GRIDS) && (render_engine->render_type != RENDER_NO_GRIDS_AND_ORIGINS))
		{
			glUniform1i(color_loc, 0);
			glBindVertexArray(render_engine->grid.grid_VAO);
			glDrawArrays(GL_LINES, 0, render_engine->vertices_count[0] / 2);
		}
		handle_key_press(window, render_engine, frame_info);
		mouse_panning(window, cursor_state);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void destroyPlotter(Plotter* plotter)
{
	free(plotter->render_engine);
	free(plotter->frame_info);
	free(plotter->cursor_state);
	glfwDestroyWindow(plotter->window);
	//glfwTerminate();
}

void plotPixel(Plotter* plot_device, int x, int y)
{
	// I think I should add a function updatePixel to call update_plot when required
	setPixel(plot_device->frame_info, (Point) { x, y });
}

void updatePixel(Plotter* plot_device)
{
	glfwMakeContextCurrent(plot_device->window);
	update_plot(plot_device->render_engine, plot_device->frame_info);
}