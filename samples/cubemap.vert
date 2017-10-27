#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;
in vec4 in_BoneIndex;
in vec4 in_BoneWeight;

uniform mat4 BoneMat[128];
uniform int NumBones;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat4 ViewMat;
uniform mat4 GeomTransform;



uniform sampler2D NegXTex;
uniform sampler2D PosXTex;
uniform sampler2D NegYTex;
uniform sampler2D PosYTex;
uniform sampler2D NegZTex;
uniform sampler2D PosZTex; 
uniform sampler2D tex;

out vec2 out_TexCoord;
out vec3 out_Normal;   // normal vector (camera coordinates)
out vec3 out_CamCoord; // vertex position (camera coordinates)
out vec3 out_WorldCoord;
out vec3 out_CamPos;

void main() 
{
	// Copy texture coordinates and color to fragment program
	mat4 inverseView = inverse(ViewMat);

	mat4 actualModelView;
	if(NumBones > 0)
	{
		mat4 m = in_BoneWeight.x * BoneMat[int(in_BoneIndex.x)] +
			in_BoneWeight.y * BoneMat[int(in_BoneIndex.y)] +
			in_BoneWeight.z * BoneMat[int(in_BoneIndex.z)] +
			in_BoneWeight.w * BoneMat[int(in_BoneIndex.w)];
		actualModelView = ModelView * m;
	}
	else
		actualModelView = ModelView * GeomTransform;

	mat4 normalMat = transpose(inverse(mat4(actualModelView)));
	out_Normal = vec3((inverseView * actualModelView * vec4(in_Normal.xyz,0)).xyz);
	
	out_WorldCoord = (inverseView * actualModelView * vec4(in_Position.xyz,1)).xyz;

	out_CamPos = (inverseView * vec4(0,0,0,1)).xyz;
	// Transform normal from object coordinates to camera coordinates
	//out_Normal = normalize(inverseView * normalMat * vec4(in_Normal.xyz,1)).xyz;

	// Transform vertex from object to unhomogenized Normalized Device
	// Coordinates (NDC).
	gl_Position = Projection * actualModelView * vec4(in_Position.xyz, 1);

}
