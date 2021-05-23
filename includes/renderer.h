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


int initialize_renderer(Renderer* render_engine,frameBuffer*frame_buffer, int width, int height);

void reset_frame_buffer(frameBuffer* frame_buffer);

void compile_and_log_shaders(struct shader*, int shader_type);
int load_shader_from_file(struct shader* shaders, const char* vertex_shader_path, const char* fragment_shader_path);

// set the corresponding pixel 
// each pixel is a square whose dimension depends upon the scale factor and the height of the 
// window

// Some functions shouldn't be provided to the API user directly
void setPixel(frameBuffer* frame_buffer, Point p);

// Reset the given pixel
// For pixel deletion another data structure than array might be useful I guess
// will consider it later.. just going with array for now
void resetPixel(frameBuffer* frame_buffer, Point p);
void update_plot(Renderer* render_engine, frameBuffer* frrame_buffer);
void update_frame(Renderer* render_engine, frameBuffer* frame_buffer);


#endif