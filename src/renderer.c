#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../includes/renderer.h"

void origin_border(Renderer*, frameBuffer*);
void calculate_coordinate(float origin_x_or_y, float scale, float* left_or_bottom, float* right_or_top, int* first_count, int* second_count);


int load_shader_from_file(shader* shaders, const char* vertex_shader_path, const char* fragment_shader_path)
{
	FILE* vs, * fs;
	if (!(vs = fopen(vertex_shader_path, "r")))
	{
		fprintf(stderr, "\nFailed to open vertex shader %s.", vertex_shader_path);
		return -1;
	}
	if (!(fs = fopen(fragment_shader_path, "r")))
	{
		fprintf(stderr, "\nFailed to open fragment shader %s.", fragment_shader_path);
		return -1;
	}

	int count = 0;
	while (fgetc(vs) != EOF)
		count++;

	char* vs_source = malloc(sizeof(char) * (count + 1));
	fseek(vs, 0, SEEK_SET);

	int index = 0;
	char ch;
	while ((ch = fgetc(vs)) != EOF)
		vs_source[index++] = ch;

	assert(index <= count);
	printf("\n Index and count are : %d and %d.", index, count);
	vs_source[index] = '\0';

	shaders->vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(shaders->vertex_shader, 1, &vs_source, NULL);

	// Similarly load the fragment shaders
	count = 0;
	while (fgetc(fs) != EOF) count++;

	fseek(fs, 0, SEEK_SET);
	char* fs_source = malloc(sizeof(char) * (count + 1));
	index = 0;
	while ((ch = fgetc(fs)) != EOF)
		fs_source[index++] = ch;
	assert(index <= count);
	fs_source[index] = '\0';

	shaders->fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(shaders->fragment_shader, 1, &fs_source, NULL);

	compile_and_log_shaders(shaders, GL_VERTEX_SHADER);
	compile_and_log_shaders(shaders, GL_FRAGMENT_SHADER);
	free(fs_source);
	free(vs_source);
	fclose(fs);
	fclose(vs);
	return 0;
}

int initialize_renderer(Renderer* render_engine, frameBuffer* frame_buffer, int width, int height)
{
	shader render_shader;
	int err = load_shader_from_file(&render_shader, "./src/shaders/vertex_shader.vs", "./src/shaders/fragment_shader.fs");
	if (err == -1)
	{
		fprintf(stderr, "Failed to load shaders.");
		return -1;
	}
	// Attach the required shaders
	render_engine->shader_program = glCreateProgram();
	glAttachShader(render_engine->shader_program, render_shader.fragment_shader);
	glAttachShader(render_engine->shader_program, render_shader.vertex_shader);

	glLinkProgram(render_engine->shader_program);

	int success = 0;
	glGetProgramiv(render_engine->shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(render_engine->shader_program, 512, NULL, infoLog);
		fprintf(stderr, "\nFailed to link shader program. -> %s. Exiting", infoLog);
		return -1;
	}
	else
		fprintf(stderr, "\nPassed");
	float aspect_ratio = (float)width / height;
	// Time to fill in the data 
	// I think these need to be determined dynamically too 
	// So let's start with a scale factor of 0.02f

	frame_buffer->scale_factor = 0.1f;

	frame_buffer->origin_x = 0.0f;
	frame_buffer->origin_y = 0.0f;

	// Draw grid lines .. Nothing more now 
	// It should have suppport for panning
	// It should be scalable
	// It must remain all the points that are plotted.
	// It should be reasonably faster
	// It shouldn't leak memory and no overflows

	// This won't work .. I need some better idea .. Gotta think something
	float* vertices = malloc(sizeof(float) * ((1.0f / 0.1f + 0.5) * 2 + 1) * 4 * 2);
	int indices = 0;

	//vertices[indices++] = -1.0f;
	//vertices[indices++] = 0;
	//vertices[indices++] = 1.0f;
	//vertices[indices++] = 0;

	//for (float y = 0.1f; y <= 1.05f; y+=0.1f)
	//{
	//	vertices[indices++] = -1.0f;
	//	vertices[indices++] = y;
	//	vertices[indices++] = 1.0f;
	//	vertices[indices++] = y;
	//	vertices[indices++] = -1.0f;
	//	vertices[indices++] = -y;
	//	vertices[indices++] = 1.0f;
	//	vertices[indices++] = -y;
	//}

	//auto k = 1;
	//vertices[indices++] = 0.0f;
	//vertices[indices++] = -1.0f;
	//vertices[indices++] = 0.0f;
	//vertices[indices++] = 1.0f;

	//for (float x = 0.1f/aspect_ratio; x <= 1.05/aspect_ratio; x += 0.1f/aspect_ratio)
	//{
	//	vertices[indices++] = x;
	//	vertices[indices++] = -1.0f;
	//	vertices[indices++] = x;
	//	vertices[indices++] = 1.0f;
	//	vertices[indices++] = -x;
	//	vertices[indices++] = -1.0f;
	//	vertices[indices++] = -x;
	//	vertices[indices++] = 1.0f;
	//}


	// Start with origin and start calculating the index at which line will be visible in the frame buffer
	// if origin hasn't been shifted and is within (-1,-1) x (1,1), origin is within the screen
	// else origin is out of the screen and we should calculate the value of x and y which will start to be seen in the screen
	// Modern pipeline is making this non trivial 

	// Calculate the value of x which will be seen on the screen
	// Origin might've shifted toward right or toward left 
	// if it is in right, it will be greater than 1 else it will be less than 1
	float x_right = 0;
	float x_left = 0;
	int x_left_count = 0;
	int x_right_count = 0;

	float y_top = 0, y_bottom = 0;
	int y_top_count = 0;
	int y_bottom_count = 0;
	frame_buffer->origin_x = 0.7f;
	frame_buffer->scale_factor = 0.15;

	fprintf(stderr, "x_left and x_right determined with scale factors are : %f %f %f.\n", x_left, x_right, frame_buffer->scale_factor);
	// Draw the shifted lines 
	// Do similiar for y-coordinate now 
	
	indices = 0;

	for (float i = x_left; i <= x_right + 0.0001; i += frame_buffer->scale_factor)
	{
		vertices[indices++] = i;
		vertices[indices++] = -1.0f;
		vertices[indices++] = i;
		vertices[indices++] = 1.0f;
	}

	// Create a vertex array object
	glGenVertexArrays(1, &render_engine->origin_vertex_array);
	glBindVertexArray(render_engine->origin_vertex_array);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * ((1.0f / 0.1f + 0.5) * 2 + 1) * 4 * 2, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	// Origin layout 

	origin_border(render_engine, frame_buffer);

	// Its a trivial case to map to ndc now
	Point ndc;

	return 0;
}

void compile_and_log_shaders(shader* shaders, int shader_type)
{
	char logInfo[512];
	int success = -1;
	GLuint current_shader = 0;
	if (shader_type == GL_VERTEX_SHADER)
		current_shader = shaders->vertex_shader;
	else if (shader_type == GL_FRAGMENT_SHADER)
		current_shader = shaders->fragment_shader;
	else
	{
		fprintf(stderr, "\nError invalid shader type passed to %s.", __func__);
		return;
	}

	glCompileShader(current_shader);
	glGetShaderiv(current_shader, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		glGetShaderInfoLog(current_shader, 512, NULL, logInfo);
		if (shader_type == GL_VERTEX_SHADER)
			fprintf(stderr, "\nVertex shader compilation failed : %s.", logInfo);
		else
			fprintf(stderr, "\nFragment shader compilation failed. : %s.", logInfo);
		return;
	}

	if (shader_type == GL_VERTEX_SHADER)
		fprintf(stderr, "\nVertex shader compilation passed.");
	else
		fprintf(stderr, "\nFragment shader compilation passed.");
}

void origin_border(Renderer* render_engine, frameBuffer* frame_buffer)
{
	float aspect_ratio = 1200.0f / 800;
	float x = frame_buffer->origin_x;
	float y = frame_buffer->origin_y;
	float thick_x = frame_buffer->scale_factor / (25*aspect_ratio);
	float thick_y = frame_buffer->scale_factor / 25;
	// Draw the quad around x-axis and y-axis
	float quad_vertices[] = {
			x + thick_x, 1.0f, x - thick_x, 1.0f, x - thick_x, -1.0f,
			x + thick_x, 1.0f, x + thick_x, -1.0f, x - thick_x, -1.0f,

			1.0f, y + thick_y, 1.0f, y - thick_y, -1.0f, y + thick_y,
			1.0f, y - thick_y, -1.0f, y + thick_y, -1.0f, y - thick_y
	};
	glGenVertexArrays(1, &render_engine->scr_vertex_array);
	glBindVertexArray(render_engine->scr_vertex_array);
	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	render_engine->vertices_count[1] = 12;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

// function to calculate left, right, top, bottom co-ordinate in ndc and its respecitve cartesian co-ordinate to 
void calculate_coordinate(float origin,float x, float* left, float* right, int* first_coord, int* second_coord)
{

	if (fabs(frame_buffer->origin_x) > 1)
	{
		if (frame_buffer->origin_x > 1)
		{
			x_right = (frame_buffer->origin_x - 1) / frame_buffer->scale_factor;
			x_right_count = -ceilf(x_right);
			x_right = frame_buffer->origin_x - frame_buffer->scale_factor * (-x_right_count);

			// calculate the x_right with above information now 
			x_left = (x_right + 1) / frame_buffer->scale_factor;
			x_left_count = x_right_count - floorf(x_left);
			x_left = x_right - (int)floorf(x_left) * frame_buffer->scale_factor;
		}
		else
		{
			x_left = (-1.0f - frame_buffer->origin_x) / frame_buffer->scale_factor;
			x_left_count = ceilf(x_left);
			x_left = frame_buffer->origin_x + frame_buffer->scale_factor * x_left_count;

			// Again use above information to calculate the right co ordinate and starting position
			// It should be precise
			x_right = (1 - x_left) / frame_buffer->scale_factor;
			x_right_count = x_left_count + floorf(x_right);
			x_right = x_left + (int)floorf(x_right) * frame_buffer->scale_factor;
		}
	}
	else
	{
		// Origin is within the frame_buffer .. just calculate the x_left and x_right
		x_left = (frame_buffer->origin_x + 1.0f) / frame_buffer->scale_factor;
		x_left_count = -floorf(x_left);
		x_left = frame_buffer->origin_x - frame_buffer->scale_factor * (-x_left_count);

		x_right = (1.0f - frame_buffer->origin_x) / frame_buffer->scale_factor;
		x_right_count = floorf(x_right);
		x_right = frame_buffer->origin_x + x_right_count * frame_buffer->scale_factor;
	}

}