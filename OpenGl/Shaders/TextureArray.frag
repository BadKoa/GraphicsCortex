#<fragment shader>

#version 330 core 

layout (location = 0) out vec4 frag_color;
in vec2 tex_coords;
in vec3 tex_indicies;

uniform sampler2DArray texture_array;
void main(){
	vec4 color = texture(texture_array, vec3(tex_coords, tex_indicies.x));
	if(color.a < 0.1f)
		discard;
	frag_color = color;
 }
