#ifndef RENDERER_H
#define RENDERER_H
#include <glad/glad.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

typedef struct Point
{
	float x;
	float y;
} Point;

typedef struct shader
{
	GLuint vertex_shader;
	GLuint fragment_shader;
} shader;

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

typedef struct frameBuffer
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
}frameBuffer;


int initialize_renderer(Renderer* render_engine,frameBuffer*frame_buffer, int width, int height);

void reset_frame_buffer(frameBuffer* frame_buffer);

void compile_and_log_shaders(struct shader*, int shader_type);
int load_shader_from_file(struct shader* shaders, const char* vertex_shader_path, const char* fragment_shader_path);

// set the corresponding pixel 
// each pixel is a square whose dimension depends upon the scale factor and the height of the 
// window

void setPixel(frameBuffer* frame_buffer, Point p);

// Reset the given pixel
// For pixel deletion another data structure than array might be useful I guess
// will consider it later.. just going with array for now
void resetPixel(frameBuffer* frame_buffer, Point p);
void update_plot(Renderer* render_engine, frameBuffer* frrame_buffer);
void update_frame(Renderer* render_engine, frameBuffer* frame_buffer);


#endif