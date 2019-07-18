#version 330

layout(location = 0) in vec4 in_pos;

uniform mat4 proj;
uniform mat4 view;

uniform vec2 tex_size;
uniform vec2 sub_size;

uniform int tex_ids[128];
uniform mat4 models[128];

uniform int flip_x[128];
uniform int flip_y[128];

out vec2 o_texc;

void main(){
	vec2 m_texc = in_pos.zw;
	
	mat4 _model = models[gl_InstanceID];
	int _tex_id = tex_ids[gl_InstanceID];
	int _flip_x = flip_x[gl_InstanceID];
	int _flip_y = flip_y[gl_InstanceID];

	if(_flip_x == 1){
		m_texc.x = 1 - m_texc.x;
	}

	if(_flip_y == 1){
		m_texc.y = 1 - m_texc.y;
	}

	int per_width = int(tex_size.x / sub_size.x);
	vec2 offset = vec2(sub_size.x * (_tex_id % per_width), sub_size.y * (_tex_id / per_width));
	
	m_texc = (offset / tex_size) + (sub_size / tex_size)  * m_texc;
	o_texc = m_texc;

	gl_Position = proj * view * _model * vec4(in_pos.xy, 0.0f, 1.0f);
}
