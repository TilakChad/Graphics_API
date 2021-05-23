// We need to create an abstraction for the given graphics package 
#include "../includes/GraphicsAPI.h"

void DDA_line(frameBuffer* frame_buffer, int x1, int y1, int x2, int y2);
float absolute(float x);

float absolute(float x)
{
	if (x >= 0)
		return x;
	return -x;
}

void DDA_line(Plotter* plot_device, int x1, int y1, int x2, int y2)
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
		// It is implemented for only positive lines .. 
		plotPixel(plot_device, (int)(x + 0.5f), (int)(y + 0.5f));
		x += dx;
		y += dy;
		// printf("%f %f\n", x, y);
	}
}
//
int main()
{
	Plotter* plot_device = createPlotter(1200, 800, 0.0);
	DDA_line(plot_device, 1, 0, 7, 4);

	updatePixel(plot_device);
	// Once plotting started it can't be modified as of now.. Need to modify it to work like that though 
	// Other operations like moving, panning works as intended
	plot(plot_device);
	destroyPlotter(plot_device);
	return 0;
}