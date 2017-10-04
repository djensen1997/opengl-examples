#version 150 // Specify which version of GLSL we are using.

in vec3 in_Position; // vertex position (object coordinates)
in vec3 in_Normal;   // normal vector   (object coordinates)

out vec4 out_VertexPos; // vertex position (camera coordinates)
out vec4 out_Normal;    // normal vector   (camera coordinates)
out vec4 lightPos;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 GeomTransform;
uniform mat3 NormalMat;


void main()
{	
	vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.0);
	//calculate the position of the light
	lightPos = ModelView * GeomTransform * vec4(13.4, 250, -3.7,1);
	// Transform the normal by the NormalMat and send it to the
	// fragment program.
	mat3 NormalMat = mat3(transpose(inverse(ModelView * GeomTransform)));
	vec3 transformedNormal = normalize(NormalMat * in_Normal);
	out_Normal = vec4(transformedNormal.xyz, 0);

	// Transform the vertex position from object coordinates into
	// camera coordinates.
	out_VertexPos = ModelView * GeomTransform * vec4(in_Position, 1);

	// Transform the vertex position from object coordinates into
	// Normalized Device Coordinates (NDC).
	gl_Position = Projection * ModelView * GeomTransform * pos;

	
	
}
