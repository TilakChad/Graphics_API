#version 330 core

out vec4 color;
uniform int color_code;

void main()
{
	if(color_code == 0)
		color = vec4(0.65f,0.65f,0.65f,1.0f);
	else if (color_code == 1)
		color = vec4(1.0f,0.0f,0.0f,1.0f);
	else 
		color = vec4(0.0f,1.0f,0.0f,1.0f);
}