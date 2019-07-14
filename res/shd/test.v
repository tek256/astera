#version 330

layout(location = 0) in vec4 vert;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

out vec2 out_tc;

void main(){
	out_tc = vert.zw;

	gl_Position = proj * view * model * vec4(vert.xy, 0.f, 1.f);
}
