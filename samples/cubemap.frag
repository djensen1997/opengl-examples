#version 150 // GLSL 150 = OpenGL 3.2


uniform int HasTex;    // Is there a texture in tex?
out vec4 fragColor;
in vec3 out_Normal; // normal vector in WORLD coordinates
in vec3 out_WorldCoord; // position of fragment in WORLD coordinates
in vec3 out_CamPos; // position of camera in WORLD coordinates
in vec3 out_CamCoord;
in vec2 in_TexCoord;

uniform sampler2D tex; // texture from the model (not needed for this assignment!)
uniform sampler2D NegXTex;
uniform sampler2D PosXTex;
uniform sampler2D NegYTex;
uniform sampler2D PosYTex;
uniform sampler2D NegZTex;
uniform sampler2D PosZTex; 


void main() 
{
	/* Get position of light in camera coordinates. When we do
	 * headlight style rendering, the light will be at the position of
	 * the camera! */
	//vec3 lightPos = vec3(0,0,0);
	vec3 incidentVec = normalize(vec3(out_WorldCoord) - vec3(out_CamPos)); //normalized reflection vector
	vec3 norm_normal = normalize(out_Normal); //normalized world normal


	/* Calculate a vector pointing from our current position (in
	 * camera coordinates) to the light position. */
	//vec3 lightDir = lightPos - out_CamCoord.xyz;

	/* Calculate diffuse shading */
	//float diffuse = diffuseScalar(out_Normal, lightDir, true);
	vec3 reflectedVec = reflect(incidentVec,norm_normal);

	float x = abs(reflectedVec.x);
	float y = abs(reflectedVec.y);
	float z = abs(reflectedVec.z);

	if(x > y && x > z){
		if(reflectedVec.x < 0){
			fragColor = texture(NegXTex, vec2(-reflectedVec.z,reflectedVec.y)/reflectedVec.x);
		}else{
			fragColor = texture(PosXTex, vec2(reflectedVec.z,reflectedVec.y)/reflectedVec.x);
		}
	}else if(y > z && y > x){
		if(reflectedVec.y < 0){
			fragColor = texture(NegYTex, vec2(reflectedVec.x, reflectedVec.z)/reflectedVec.y);
		}else{
			fragColor = texture(PosYTex, vec2(-reflectedVec.x, reflectedVec.z)/reflectedVec.y);
		}	
	}else if(z > y && z > x){
		if(reflectedVec.z < 0){
			fragColor = texture(NegZTex, vec2(-reflectedVec.x, reflectedVec.y)/reflectedVec.z);
		}else{
			fragColor = texture(PosZTex, vec2(reflectedVec.x, reflectedVec.y)/reflectedVec.z);
		}
	}else{
		fragColor.xyz = vec3(.2,.2,.2);
	}

	//fragColor.xyz = vec3(.2,.2,.2);

}
