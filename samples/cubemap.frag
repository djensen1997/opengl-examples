#version 150 // GLSL 150 = OpenGL 3.2


uniform int HasTex;    // Is there a texture in tex?
out vec4 fragColor;
in vec3 out_Normal; // normal vector in WORLD coordinates
in vec3 out_WorldCoord; // position of fragment in WORLD coordinates
in vec3 out_CamPos; // position of camera in WORLD coordinates
in vec3 out_CamCoord;

uniform sampler2D tex; // texture from the model (not needed for this assignment!)
uniform sampler2D NegXTex;
uniform sampler2D PosXTex;
uniform sampler2D NegYTex;
uniform sampler2D PosYTex;
uniform sampler2D NegZTex;
uniform sampler2D PosZTex; 


/** Calculate diffuse shading. Normal and light direction do not need
 * to be normalized. */
float diffuseScalar(vec3 normal, vec3 lightDir, bool frontBackSame)
{
	/* Basic equation for diffuse shading */
	float diffuse = dot(normalize(lightDir), normalize(normal.xyz));

	/* The diffuse value will be negative if the normal is pointing in
	 * the opposite direction of the light. Set diffuse to 0 in this
	 * case. Alternatively, we could take the absolute value to light
	 * the front and back the same way. Either way, diffuse should now
	 * be a value from 0 to 1. */
	if(frontBackSame)
		diffuse = abs(diffuse);
	else
		diffuse = clamp(diffuse, 0, 1);

	/* Keep diffuse value in range from .5 to 1 to prevent any object
	 * from appearing too dark. Not technically part of diffuse
	 * shading---however, you may like the appearance of this. */
	diffuse = diffuse/2 + .5;

	return diffuse;
}


void main() 
{
	/* Get position of light in camera coordinates. When we do
	 * headlight style rendering, the light will be at the position of
	 * the camera! */
	vec3 lightPos = vec3(0,0,0);
	vec3 reflectionVec = normalize(vec3(out_WorldCoord) - vec3(out_CamPos)); //normalized reflection vector
	vec3 norm_normal = normalize(out_Normal); //normalized world normal


	/* Calculate a vector pointing from our current position (in
	 * camera coordinates) to the light position. */
	//vec3 lightDir = lightPos - out_CamCoord.xyz;

	/* Calculate diffuse shading */
	//float diffuse = diffuseScalar(out_Normal, lightDir, true);
	reflect(reflectionVec, norm_normal);
	fragColor.xyz = /*(vec4((reflectionVec+1)/2,1) **/ vec4(1.0, 0.2, 0.1, 1.0).xyz;
	
}
