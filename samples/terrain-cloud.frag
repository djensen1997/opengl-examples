#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;

/* These "in" variables are interpolated from the values calculated
 * per vertex in the vertex program. They are the output of our vertex
 * program. For this program, I have named the variables so they are
 * correct in the vertex program---and they must have the same name in
 * the fragment program. */
in vec4 out_VertexPos; /* Fragment position (in camera
                        * coordinates). Although it is named
                        * VertexPos, is actually the fragment position
                        * because it has been interpolated across the
                        * face of the triangle. */
in vec4 out_Normal;    // normal vector   (camera coordinates)

uniform int red;
uniform int green;
uniform sampler2D clouds;
in vec2 out_TexCoord;


void main() 
{
	

	vec3 color = texture(clouds, out_TexCoord).xyz;
	float scalar = (color.x + color.y + color.z)/3;
	fragColor = vec4(1,1,1,scalar);
}
