#version 330

layout(location = 0) in vec4 vert;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

uniform vec2 tex_size;
uniform vec2 sub_size;

uniform int tex_id;

out vec2 out_tc;

void main(){
	vec2 m_texc = vert.zw;
	int _tex_id = tex_id;

	int per_width = int(tex_size.x / sub_size.x);
	vec2 offset = vec2(sub_size.x * (_tex_id % per_width), sub_size.y * (_tex_id / per_width));
	
	m_texc = (offset / tex_size) + (sub_size / tex_size)  * m_texc;
	out_tc = m_texc;

	gl_Position = proj * view * model * vec4(vert.xy, 0.f, 1.f);
}
