#version 150 // GLSL 150 = OpenGL 3.2

out vec4 fragColor;
in vec3 lightDir;
in vec3 out_Normal;
uniform float color;

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
	float diffuse = diffuseScalar(out_Normal,lightDir,false);
	//0 is for grey
	if(color == 0.0){
		fragColor = vec4(0.6,0.6,0.6,1) * diffuse;
	//1 is for black
	}else if(color == 1.0){
		fragColor = vec4(0.1,0.1,0.1,1) * diffuse;
	//2 is for green
	}else if(color == 2.0){
		fragColor = vec4(0.2,0.8,0.2,1) * diffuse;
	}else{
		fragColor = vec4(0.8,0.2,0.2,1) * diffuse;
	}
}
