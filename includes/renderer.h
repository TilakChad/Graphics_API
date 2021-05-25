#ifndef RENDERER_H
#define RENDERER_H
#include <glad/glad.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "abstract.h"
typedef struct shader
{
	GLuint vertex_shader;
	GLuint fragment_shader;
} shader;


int initialize_renderer(Renderer* render_engine,viewInfo*frame_info, int width, int height);

void reset_frame_info(viewInfo* frame_info);

void compile_and_log_shaders(struct shader*, int shader_type);
int load_shader_from_file(struct shader* shaders, const char* vertex_shader_path, const char* fragment_shader_path);

// set the corresponding pixel 
// each pixel is a square whose dimension depends upon the scale factor and the height of the 
// window

// Some functions shouldn't be provided to the API user directly
void setPixel(viewInfo* frame_info, Point p);

// Reset the given pixel
// For pixel deletion another data structure than array might be useful I guess
// will consider it later.. just going with array for now
void resetPixel(viewInfo* frame_info, Point p);
void update_plot(Renderer* render_engine, viewInfo* frrame_buffer);
void update_frame(Renderer* render_engine, viewInfo* frame_info);


#endif