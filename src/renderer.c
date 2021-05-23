#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../includes/renderer.h"

static void origin_border(Renderer*, frameBuffer*);
static void update_graph(Renderer* render_engine, frameBuffer* frame_buffer);
void update_plot(Renderer* render_engine, frameBuffer* frrame_buffer);

void update_frame(Renderer* render_engine, frameBuffer* frame_buffer);

static void calculate_coordinate(float origin_x_or_y, float scale, float* left_or_bottom, float* right_or_top, int* first_count, int* second_count);
static int allocate_more(frameBuffer* frame_buffer);
// Hehe.. can't simply pass the plotted_points struct cause it has already been declared anonymously 
static bool contains(frameBuffer* frame_buffer,Point p);

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
	frame_buffer->aspect_ratio = aspect_ratio;
	// Time to fill in the data 
	// I think these need to be determined dynamically too 
	// So let's start with a scale factor of 0.02f

	frame_buffer->scale_factor = 0.25f;
	frame_buffer->origin_x = 0.0f;
	frame_buffer->origin_y = 0.0f;

	for (int i = 0; i < 3; ++i) 
		render_engine->vertices_count[i] = 0;

	// Origin layout 
	update_graph(render_engine, frame_buffer);
	origin_border(render_engine, frame_buffer);

	// Initialize the plotted pointy thingy
	frame_buffer->plotted_points.size = 0;
	frame_buffer->plotted_points.capacity = 0;

	// Lets allocate a capacity for 1000 points for now 
	frame_buffer->plotted_points.points = malloc(sizeof(Point) * 1000);
	frame_buffer->plotted_points.capacity = 1000;

	// Now update the points that need to be plotted and set by the user ...
	// Initialize the plot detail properly
	render_engine->plot.contain_VBO = false;
	render_engine->grid.contain_VBO = false;
	render_engine->origin.contain_VBO = false;

	update_plot(render_engine, frame_buffer);
	return 0;
}


void origin_border(Renderer* render_engine, frameBuffer* frame_buffer)
{
	float aspect_ratio = frame_buffer->aspect_ratio;
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

	if (render_engine->origin.contain_VBO)
	{
		glDeleteVertexArrays(1,&render_engine->origin.origin_VAO);
		glDeleteBuffers(1,&render_engine->origin.origin_VBO);
	}

	render_engine->origin.contain_VBO = true;

	glGenVertexArrays(1, &render_engine->origin.origin_VAO);
	glBindVertexArray(render_engine->origin.origin_VAO);

	glGenBuffers(1, &render_engine->origin.origin_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, render_engine->origin.origin_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	render_engine->vertices_count[1] = 24;
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

// function to calculate left, right, top, bottom co-ordinate in ndc and its respecitve cartesian co-ordinate to 
void calculate_coordinate(float origin, float scale, float* left, float* right, int* first_coord, int* second_coord)
{
	float left_ndc = 0;
	float right_ndc = 0;
	int left_coord = 0;
	int right_coord = 0;

	if (fabs(origin) > 1)
	{
		if (origin > 1)
		{
			right_ndc = (origin - 1) / scale;
			right_coord = -ceilf(right_ndc);
			right_ndc = origin - scale * (-right_coord);

			// calculate the right_ndc and right_coord with above information now 
			left_ndc = (right_ndc + 1) / scale;
			left_coord = right_coord - floorf(left_ndc);
			left_ndc = right_ndc - (int)floorf(left_ndc) * scale;
		}
		else
		{
			left_ndc = (-1.0f - origin) / scale;
			left_coord = ceilf(left_ndc);
			left_ndc = origin + scale * left_coord;

			// Again use above information to calculate the right co ordinate and starting position
			// It should be precise
			right_ndc = (1 - left_ndc) / scale;
			right_coord = left_coord + floorf(right_ndc);
			right_ndc = left_ndc + (int)floorf(right_ndc) * scale;
		}
	}
	else
	{
		// Origin is within the frame_buffer .. just calculate the x_left and x_right
		left_ndc = (origin + 1.0f) / scale;
		left_coord = -floorf(left_ndc);
		left_ndc = origin - scale * (-left_coord);

		right_ndc = (1.0f - origin) / scale;
		right_coord = floorf(right_ndc);
		right_ndc = origin + right_coord * scale;
	}
	*left = left_ndc;
	*first_coord = left_coord;
	*right = right_ndc;
	*second_coord = right_coord;
}

void update_graph(Renderer* render_engine, frameBuffer* frame_buffer)
{
	float aspect_ratio = frame_buffer->aspect_ratio;
	// Draw grid lines .. Nothing more now 
// It should have suppport for panning
// It should be scalable
// It must remain all the points that are plotted.
// It should be reasonably faster
// It shouldn't leak memory and no overflows

// This won't work .. I need some better idea .. Gotta think something
// Lets defer the allocation of memory after all vertices are covered. It shouldn't be exact. 
	// float* vertices = malloc(sizeof(float) * ((1.0f / 0.1f + 0.5) * 2 + 1) * 4 * 4 * 2);
	int indices = 0;
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

	calculate_coordinate(frame_buffer->origin_x, frame_buffer->scale_factor / aspect_ratio, &x_left, &x_right, &x_left_count, &x_right_count);

	fprintf(stderr, "x_left and x_right determined with scale factors are : %f %f %f.\n", x_left, x_right, frame_buffer->scale_factor);
	// Draw the shifted lines 
	// Do similiar for y-coordinate now 

	calculate_coordinate(frame_buffer->origin_y, frame_buffer->scale_factor, &y_bottom, &y_top, &y_bottom_count, &y_top_count);

	fprintf(stderr, "\ny_bottom and y_top determined are : %f %f.", y_bottom, y_top);

	// Good it works now .. Now I am gonna implement set pixel function
	// A limited frame buffer isn't the option here .. so might use a array of point that will record set pixel
	// and render the pixel once they are within the visible viewport

	indices = 0;
	int count_lines = ceilf((x_right - x_left) / (frame_buffer->scale_factor / aspect_ratio)) + 1 ; // May overflow + 100 for caution .. haha funny way
	count_lines += ceilf((y_top - y_bottom) / frame_buffer->scale_factor) + 1 ; // Don't know when will this overflow due to floating points

	// Allocate enough memory for vertices 
	float* vertices = malloc(sizeof(float) * count_lines * 4 * 2);
	for (float x = x_left; x-x_right < 0.00001f ; x += frame_buffer->scale_factor / aspect_ratio)
	{
			vertices[indices++] = x;
			vertices[indices++] = -1.0f;
			vertices[indices++] = x;
			vertices[indices++] = 1.0f;
	}

	int i = 0;
	for (float y = y_bottom; y-y_top < 0.00001f ; y += frame_buffer->scale_factor)
	{
			vertices[indices++] = -1.0f;
			vertices[indices++] = y;
			vertices[indices++] = 1.0f;
			vertices[indices++] = y;
	}
	printf("\nTotal indices were : %d and total line counts were %d.", indices, count_lines);

	// Create a vertex array object
	// Check if the buffer has already been allocated

	if (render_engine->grid.contain_VBO)
	{
		glDeleteVertexArrays(1, &render_engine->grid.grid_VAO);
		glDeleteBuffers(1, &render_engine->grid.grid_VBO);
	}
	// Mark it for further deletion
	render_engine->grid.contain_VBO = true;
	glGenVertexArrays(1, &render_engine->grid.grid_VAO);
	glBindVertexArray(render_engine->grid.grid_VAO);

	glGenBuffers(1, &render_engine->grid.grid_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, render_engine->grid.grid_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (count_lines * 4) * 2, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
	render_engine->vertices_count[0] = indices;

	// Completely forgot to update the information about current plotting range to frame_buffer
	frame_buffer->plot_info.bottom = y_bottom;
	frame_buffer->plot_info.top = y_top;
	frame_buffer->plot_info.left = x_left;
	frame_buffer->plot_info.right = x_right;

	frame_buffer->plot_info.left_coord = x_left_count;
	frame_buffer->plot_info.right_coord = x_right_count;
	frame_buffer->plot_info.bottom_coord = y_bottom_count;
	frame_buffer->plot_info.top_coord = y_top_count;

	printf("\n\nY_bottom and Y-top are :-> %d %d.\n", y_bottom_count, y_top_count);

	// Freed memory
	free(vertices);
}

void update_plot(Renderer* render_engine, frameBuffer* frame_buffer)
{
	float aspect_ratio = frame_buffer->aspect_ratio;
	if (render_engine->plot.contain_VBO)
	{
		glDeleteBuffers(1, &render_engine->plot.plot_VBO);
		glDeleteVertexArrays(1, &render_engine->plot.plot_VAO);
	}

	// Set it so that in next turn gpu memory can be freed safely
	render_engine->plot.contain_VBO = true; 

	glGenVertexArrays(1, &render_engine->plot.plot_VAO);
	glBindVertexArray(render_engine->plot.plot_VAO);

	glGenBuffers(1, &render_engine->plot.plot_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, render_engine->plot.plot_VBO);

	float origin_x = frame_buffer->origin_x;
	float origin_y = frame_buffer->origin_y;
	float scale = frame_buffer->scale_factor;

	//fprintf(stderr, "\nOrigin and scale factor are -> %f %f -> %f.",origin_x, origin_y, scale);

	/*
	*
(x,y+1)	_____ (x+1,y+1)
	   |	 |
	   |(x,y)|
	   |_____|
	(x,y)     (x+1,y)
	*
	*/

	// float vertices[] = { x , y, x + 1 * scale / aspect_ratio, y, x + 1 * scale / aspect_ratio,y + 1 * scale,
	//					 x + 1 * scale / aspect_ratio,y + 1 * scale, x,y + 1 * scale,x,y };

	// Need to allocate enough memory 
	// Each point is going to take 6 vertices 
	// Ok that's it 
	// +ve x-axis will contain non-negative numbers
	// -ve x-axis will contain negative numbers
	// Let's do it Math way 

	// Let's not implement out-of-frame origin for now 

	// float* pixel_vertices = malloc(sizeof(float) * 6 * 2 * frame_buffer->plotted_points.size);

	Point* points = frame_buffer->plotted_points.points;
	//if (frame_buffer->plotted_points.size)
	//	fprintf(stderr, "\nPoint to be plotted is %f %f.", points[0].x, points[0].y);
	int indices = 0;

	// For efficient plotting let's fix the plot criteria
	// Points are to be plotted if they are in visible area of the screen
	// Various factors like panning, moving or scaling might change the co-ordinates that are currently visible in the area
	// so analyze each point and plot it only if it is within the current visible scren

	// fprintf(stderr, "\n\nValues are : %d %d and %d %d.", frame_buffer->plot_info.left_coord, frame_buffer->plot_info.right_coord, frame_buffer->plot_info.bottom_coord, frame_buffer->plot_info.top_coord);

	// Again might need to do a double pass to allocate points that are within the visible region

	int visible_pixels = 0;
	for (unsigned int i = 0; i < frame_buffer->plotted_points.size; ++i)
	{
		if (!
			((points[i].x < frame_buffer->plot_info.left_coord - 1) ||
				(points[i].x > frame_buffer->plot_info.right_coord) ||
				(points[i].y < frame_buffer->plot_info.bottom_coord - 1) ||
				(points[i].y > frame_buffer->plot_info.top_coord)))
		{
			visible_pixels++;
		}
	}
	// Allocate the memory accordingly ... Don't waste more memory 
	float* pixel_vertices = malloc(sizeof(float) * visible_pixels * 6 * 2);

	for (unsigned int i = 0; i < frame_buffer->plotted_points.size; ++i)
	{
		// Looks ugly, righ? 
		// Go ahead and use the De-Morgans law on the below boolean expression
		if (!
			((points[i].x < frame_buffer->plot_info.left_coord - 1) ||
			(points[i].x > frame_buffer->plot_info.right_coord) ||
			(points[i].y < frame_buffer->plot_info.bottom_coord - 1) ||
			(points[i].y > frame_buffer->plot_info.top_coord)))
		{

			pixel_vertices[indices++] = origin_x + points[i].x * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + (points[i].y + 1) * scale;
			pixel_vertices[indices++] = origin_x + (points[i].x + 1) * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + (points[i].y + 1) * scale;
			pixel_vertices[indices++] = origin_x + points[i].x * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + points[i].y * scale;


			pixel_vertices[indices++] = origin_x + points[i].x * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + points[i].y * scale;
			pixel_vertices[indices++] = origin_x + (points[i].x + 1) * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + (points[i].y + 1) * scale;
			pixel_vertices[indices++] = origin_x + (points[i].x + 1) * scale / aspect_ratio;
			pixel_vertices[indices++] = origin_y + points[i].y * scale;
		}
	}

	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2 * visible_pixels, pixel_vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	render_engine->vertices_count[2] = indices;
	fprintf(stderr, "\n\nValue of the indices are : %d and total visible pixels are %d.", indices, visible_pixels);
	free(pixel_vertices);
}

void update_frame(Renderer* render_engine, frameBuffer* frame_buffer)
{
	origin_border(render_engine, frame_buffer);
	update_graph(render_engine, frame_buffer);
	update_plot(render_engine, frame_buffer);
}

void setPixel(frameBuffer* frame_buffer, Point p)
{
	// check if the point storage have enough allocated memory for further adding point
	if (frame_buffer->plotted_points.size == frame_buffer->plotted_points.capacity)
	{
		int err_code = allocate_more(frame_buffer);
		if (err_code)
		{
			fprintf(stderr, "\nFailed to allocate more memory. Returning ..");
			return;
		}
	}

	// Now check if the given point is already available
	// Let's build a contain interface in case I decided to change the underlying data structure

	if (!contains(frame_buffer, p))
	{
		frame_buffer->plotted_points.points[frame_buffer->plotted_points.size++] = p;
	}
	
	// If contains don't do anything 
	// Hmm.. I think I shouldn't be allocating more if point already contains .. It shouldn't matter much but will fix it later
	// Maybe someday, a single point which already contain will allocate 16 MB more memory for nothing :D :D 
}

void resetPixel(frameBuffer* frame_buffer, Point p)
{
	// Currently not implementing this for now 
}

int allocate_more(frameBuffer* frame_buffer)
{
	// Double the capacity of the previous storage
	// Maintain a pointer to previous storage

	Point* previous = frame_buffer->plotted_points.points;

	// Re allocate the current one
	frame_buffer->plotted_points.points = malloc(sizeof(Point) * (frame_buffer->plotted_points.capacity * 2));
	if (!frame_buffer->plotted_points.points)
	{
		fprintf(stderr, "\nFailed to allocate new memory for points .. ");
		return -1;
	}
	frame_buffer->plotted_points.capacity <<= 1;

	// Now copy the old data into the new one 
	memcpy(frame_buffer->plotted_points.points, previous, sizeof(Point) * frame_buffer->plotted_points.size);

	// Release the previously allocated memory
	free(previous);
	return 0;
}

bool contains(frameBuffer* frame_buffer, Point p)
{
	// Use naive linear search
	// But this should do for now ..  A little optimized 
	for (unsigned int i = 0; i < frame_buffer->plotted_points.size; ++i)
	{
		// Depending upon the probability, x-axis would have more grid than y-axis.. so x-axis would have less chance of collision
		// So let's check for x component equality first
		if (frame_buffer->plotted_points.points[i].x == p.x)
		{
			// check for y component now
			if (frame_buffer->plotted_points.points[i].y == p.y)
				return true;
		}
	}
	return false;
}