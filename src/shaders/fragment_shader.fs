#version 330 core

out vec4 color;
uniform int color_code;

void main()
{
	if(color_code == 0)
		color = vec4(0.6f,0.6f,0.6f,1.0f);
	else 
		color = vec4(1.0f,0.0f,0.0f,1.0f);
}