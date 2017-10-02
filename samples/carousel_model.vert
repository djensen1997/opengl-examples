#version 150 // Specify which version of GLSL we are using.

in vec3 in_Position;
in vec3 in_Normal;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 GeomTransform;

out vec3 out_Normal;

void main()
{
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	mat3 NormalMat = mat3(transpose(inverse(ModelView * GeomTransform)));
	out_Normal = normalize(NormalMat * in_Normal);

	gl_Position = Projection * ModelView * GeomTransform * pos; // object -> unhomogenized NDC
}
