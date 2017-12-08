#version 150 // Specify which version of GLSL we are using.

out vec4 fragColor;
in vec4 out_Normal;
in vec4 out_VertexPos;
in vec4 lightPos;

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
	/* Calculate a vector pointing from our current position (in
	 * camera coordinates) to the light position. */
	vec3 vert_to_light = lightPos.xyz - vec3(out_VertexPos.xyz);
	vec3 vert_to_cam = vec3(0,0,0) - vec3(out_VertexPos.xyz);

	vec3 norm_vtl = normalize(vert_to_light);
	vec3 norm_vtc = normalize(vert_to_cam);
	vec3 H = normalize(norm_vtl + norm_vtc);

	float specular = dot(H, out_Normal.xyz);

	if(specular < 0)
		specular = 0;
	
	specular = pow(specular,10);

	/* Calculate diffuse shading */
	float diffuse = diffuseScalar(out_Normal.xyz, lightPos.xyz, true);

	/*	
	 *	fragColor.xyz = (specular + (diffuse/2+.5))*vec3(.5, .5, .5);
	 *	fragColor.a = 1;
	*/
	fragColor.xyz = (specular + (diffuse))*vec3(.5,.5,.5);
	fragColor.a = 1;
}
