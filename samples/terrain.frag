#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in vec2 out_TexCoord;
in vec3 out_Normal;
in vec3 out_lightDir;

uniform sampler2D base_terrain;
uniform sampler2D color_terrain;


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
	if(diffuse < .2){
		diffuse = .2;
	}
	diffuse = diffuse/2 + .5;

	return diffuse;
}

void main() 
{
	if(true)
	{
		float diffuse = diffuseScalar(out_Normal, out_lightDir, true);
		fragColor = texture(color_terrain, out_TexCoord);
		fragColor = diffuse * fragColor;
		fragColor.a = 1;
	}
	else
	{
		fragColor = vec4(out_Normal.xyz,1);
	}
}
