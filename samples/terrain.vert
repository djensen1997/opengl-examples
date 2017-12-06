#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 in_Normal;

uniform mat4 ModelView;
uniform mat4 Projection;
uniform sampler2D base_terrain;
uniform sampler2D color_terrain;

out vec2 out_TexCoord;
out vec3 normal;
out vec3 out_Normal;
out vec3 out_lightDir;

float calc_height(vec3 color){
	float z = color.x + color.y + color.z;
	z = z / 10;
	if(z < .02){
		z = .02;
	}else if (z > .3){
		z = .3;
	}
	return z;
}

void main(){

	vec3 lightPosition = vec3(0,10,0);
	vec4 color = texture(base_terrain,in_TexCoord);
	vec2 left_TexCoord 	= 	vec2(in_TexCoord.x - (float(1)/float(2048)), in_TexCoord.y);
	vec2 up_TexCoord 	= 	vec2(in_TexCoord.x, in_TexCoord.y - float(1)/float(1024));

	vec4 pos 		= 	vec4(in_Position.x, 	in_Position.y, 	calc_height(color.xyz), 1.0);

	vec3 left_pos 	= 	vec3(in_Position.x-1, 	in_Position.y, 	calc_height(texture(base_terrain, left_TexCoord).xyz));
	vec3 up_pos 	= 	vec3(in_Position.x, 	in_Position.y-1,calc_height(texture(base_terrain,   up_TexCoord).xyz));
	//vec4 color = vec4(1,1,1,0);

	//based on sudo code from wikipedia.org
	vec3 U = left_pos - pos.xyz;
	vec3 V = up_pos - pos.xyz;
	out_Normal.x = -(U.y * V.z - U.z * V.y);
	out_Normal.y = -(U.z * V.x - U.x * V.z);
	out_Normal.z = U.x * V.y - U.y * V.x; 
	
	mat3 NormalMatrix = transpose(inverse(mat3(ModelView)));
	out_Normal = NormalMatrix * normalize(out_Normal).xyz;
	out_lightDir = lightPosition - pos.xyz;

	out_TexCoord = in_TexCoord;
	gl_Position = Projection * ModelView * pos;

}
