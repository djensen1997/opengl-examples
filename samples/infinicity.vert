#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec3 in_Normal;
out vec3 lightDir;
out vec3 out_Normal;
out float out_Color;
in float color;

uniform mat4 ModelView;
uniform mat4 Projection;

void main() 
{
	vec3 lightPos = vec3(0,50,0);
	mat3 NormalMatrix = transpose(inverse(mat3(ModelView)));
	out_Normal = NormalMatrix * normalize(in_Normal).xyz;
	out_Color = color;

	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	pos = Projection * ModelView * pos;
	lightDir = lightPos - vec3(pos.xyz);
	gl_Position = pos;
}
