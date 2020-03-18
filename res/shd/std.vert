#version 330

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_tex;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 pass_texcoord;

void main(){
	pass_texcoord = in_tex;	

	gl_Position = proj * view * model * vec4(in_pos, 1.0f);
}
