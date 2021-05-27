#ifndef GRAPHICS_API_H
#define GRAPHICS_API_H

// I am learning again and again that glad should be included before GLFW
#include "abstract.h"
#include <GLFW/glfw3.h>

// GLFW API provide a function with which we can set a single pointer and retrieve it wherever we desire
// But we may need to pass more pointers sometimes.
// So, wrapping the pointers within a struct might be the good choice. At least, that's what I am going to do.
// Its the struct whose pointer we are going to retrieve in the callback function.


typedef struct mouseState
{
	double xpos, ypos;
	bool is_was_pressed; // :D 
} mouseState;

typedef struct Plotter
{
	Renderer* render_engine;
	viewInfo* frame_info;
	mouseState* cursor_state;
	GLFWwindow* window;
	// That should be enough
} Plotter;

// Leave scale to zero for default value 
// Only these functions will now be provided. Others will be abstracted due to abstract.h
Plotter* createPlotter(int width, int height, float scale);
void plot(Plotter*);
void destroyPlotter(Plotter*);
void plotPixel(Plotter*,int x, int y);
void updatePixel(Plotter*);
void setRenderOptions(Plotter* plot_device, unsigned char flag);
#endif 
