// We need to create an abstraction for the given graphics package 
#include "../includes/GraphicsAPI.h"

void DDA_line(Plotter* plot_device, int x1, int y1, int x2, int y2);
float absolute(float x);

float absolute(float x)
{
	if (x >= 0)
		return x;
	return -x;
}

int abs(int x)
{
	return x >= 0 ? x : -x;
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

void BressenhamLine(Plotter* plot_device, int x1, int y1, int x2, int y2)
{
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int xinc = 1, yinc = 1;

	if (x2 < x1) 
		xinc = -1;
	if (y2 < y1) 
		yinc = -1; 

	int x = x1, y = y1;

	plotPixel(plot_device, x, y);
	if (dy < dx)
	{
		// case with slope < 1
		int p = 2 * dy - dx; 
		int i = 0;
		while (i++ < dx)
		{
			if (p < 0)
			{
				x = x + xinc;
				p += 2 * dy;
			}
			else
			{
				x = x + xinc;
				y = y + yinc;
				p += 2 * dy - 2 * dx;
			}
			plotPixel(plot_device, x, y);
		}
	}
	else
	{
		int p = 2 * dx - dy;
		int i = 0;
		while (i++ < dy)
		{
			if (p < 0)
			{
				y = y + yinc;
				p += 2 * dx;
			}
			else
			{
				x = x + xinc;
				y = y + yinc;
				p += 2 * dx - 2 * dy;
			}
			plotPixel(plot_device, x, y);
		}
	}
}

int main()
{
	Plotter* plot_device = createPlotter(1200, 800, 0.0);

	Plotter* plot_device_b = createPlotter(1200, 800, 0, 0);
	// DDA_line(plot_device, 1, 0, 7, 4);
	BressenhamLine(plot_device, -1, -1, -7, -7);
	/*for (int i = -3; i < 5; ++i)
	{
		for (int j = -3; j < 5; ++j)
		{
			plotPixel(plot_device_b, i, j);
		}
	}*/

	// Available options are RENDER_ALL, NO_GRIDS, NO_PIXELS, NO_ORIGIN
	setRenderOptions(plot_device, NO_GRIDS);
	BressenhamLine(plot_device_b, 1, 0, 4, 7);

	updatePixel(plot_device);
	// Once plotting started it can't be modified as of now.. Need to modify it to work like that though 
	// Other operations like moving, panning works as intended
	plot(plot_device);
	destroyPlotter(plot_device);

	updatePixel(plot_device_b);
	plot(plot_device_b);
	destroyPlotter(plot_device_b);
	return 0;
}