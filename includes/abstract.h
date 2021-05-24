// This header file is created to abstract some functions so that they shouldn't be available
// to the end user directly. 
// This header will include common functions and typedef that would be shared between renderer, main and API

#ifndef ABSTRACT_H
#define ABSTRACT_H
#include <glad/glad.h>
#include <stdbool.h>

typedef struct Point
{
	float x;
	float y;
} Point;

typedef struct Renderer
{
	// Could've made a different struct instead of using anonymous.. but its ok. It was a later addition .. so didn't consider it changing

	struct
	{
		bool contain_VBO;
		GLuint grid_VBO;
		GLuint grid_VAO;
	} grid;

	struct
	{
		bool contain_VBO;
		GLuint origin_VBO;
		GLuint origin_VAO;
	} origin;

	GLuint shader_program;
	GLuint vertices_count[3];
	// ok let's include them here too
	struct
	{
		bool contain_VBO; // Check this flag before deleting the bound VBO.. be safe.. Its functional
		GLuint plot_VBO;
		GLuint plot_VAO; // Not going to use element array now .. And using the same set of shaders for now
	} plot;

	// it's verbose but its ok for now 
	enum
	{
		RENDER_ALL, RENDER_NO_GRIDS, RENDER_NO_ORIGIN, RENDER_NO_PIXELS, RENDER_NO_GRIDS_AND_ORIGINS
	} render_type;
} Renderer;

typedef struct viewInfo
{
	GLfloat origin_x;
	GLfloat origin_y;
	/*Point scale_factor;*/
	// scale factor is going to be same.. i.e every pixel will be perfect square
	GLfloat scale_factor;
	GLfloat aspect_ratio;
	struct
	{
		unsigned int size;
		unsigned int capacity;
		Point* points;
	} plotted_points;

	struct
	{
		float left, right, top, bottom;
		int left_coord, right_coord, top_coord, bottom_coord;
	}plot_info;
}viewInfo;

#endif
