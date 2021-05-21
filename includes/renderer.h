#ifndef RENDERER_H
#define RENDERER_H
#include <glad/glad.h>
#include <assert.h>

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
	GLuint scr_vertex_array;
	GLuint origin_vertex_array;
	GLuint shader_program;
	GLuint vertices_count[3];
} Renderer;

typedef struct frameBuffer
{
	GLfloat origin_x;
	GLfloat origin_y;
	GLuint* frame;
	GLuint width;
	GLuint height;
	/*Point scale_factor;*/
	// scale factor is going to be same.. i.e every pixel will be perfect square
	GLfloat scale_factor;
}frameBuffer;


int initialize_renderer(Renderer* render_engine,frameBuffer*frame_buffer, int width, int height);

void reset_frame_buffer(frameBuffer* frame_buffer);

void compile_and_log_shaders(struct shader*, int shader_type);
int load_shader_from_file(struct shader* shaders, const char* vertex_shader_path, const char* fragment_shader_path);
#endif