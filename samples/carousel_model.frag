#version 150 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec3 out_Normal;

void main() 
{
	fragColor.xyz = (normalize(out_Normal) + 1) / 2;
	fragColor.a = 1;
}
