#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in vec2 out_TexCoord;

uniform sampler2D base_terrain;
uniform sampler2D color_terrain;
uniform sampler2D clouds;

void main() 
{
	if(true)
	{
		fragColor = texture(color_terrain, out_TexCoord);
	}
	else
	{
		fragColor = vec4(out_TexCoord,0,1);
	}
}
