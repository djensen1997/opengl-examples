#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec2 in_TexCoord;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform sampler2D base_terrain;
uniform sampler2D color_terrain;
uniform sampler2D clouds;

out vec2 out_TexCoord;

void main() 
{
	vec4 color = texture(base_terrain,out_TexCoord);

	float z = dot(color.xyz,color.xyz);

	/**
	 *	Need to calculate z location
	 */

	vec4 pos = vec4(in_Position.x, in_Position.y, z, 1.0);
	out_TexCoord = in_TexCoord;
	gl_Position = Projection * ModelView * pos;
}
