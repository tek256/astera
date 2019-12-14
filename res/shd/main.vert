#version 330
#define MAX_BATCH_SIZE 512

layout(location = 0) in vec4 in_pos;

uniform mat4 proj;
uniform mat4 view;

uniform vec2 tex_size;
uniform vec2 sub_size;

uniform mat4 models[MAX_BATCH_SIZE];
uniform int tex_ids[MAX_BATCH_SIZE];
uniform int flip_x[MAX_BATCH_SIZE];
uniform int flip_y[MAX_BATCH_SIZE];

flat out int pass_texid;
out vec2 pass_texcoord;

void main(){
  vec2 raw_texc = in_pos.zw;
	vec2 m_texc = in_pos.zw;
	
	mat4 _model = models[gl_InstanceID];
	int _flip_x = flip_x[gl_InstanceID];
	int _flip_y = flip_y[gl_InstanceID];
  int _tex_id = tex_ids[gl_InstanceID];

	if(_flip_x == 1){
		m_texc.x = 1.0 - m_texc.x;
	}

	if(_flip_y == 1){
		m_texc.y = 1.0 - m_texc.y;
	}

  // drop 0.5 down to -0.01
  float x_fix = sign(m_texc.x - 0.51) * -0.005;
  float y_fix = sign(m_texc.y - 0.51) * -0.005;

  vec2 adj_size = sub_size;
  adj_size.x += x_fix;
  adj_size.y += y_fix;
  
  int per_width = int(tex_size.x / sub_size.x);
	vec2 offset = vec2(sub_size.x * (_tex_id % per_width), sub_size.y * (_tex_id / per_width));

  vec2 adj_offset = offset;
  adj_offset.x += x_fix;
  adj_offset.y += y_fix;

  //vec2 px_square = vec2(sub_size.x * 0.05 * sign(raw_texc.x - 0.5), sub_size.y * 0.05 * sign(raw_texc.y - 0.5));
	
	pass_texcoord = (adj_offset / tex_size) + (adj_size/ tex_size)  * m_texc;	
  pass_texid = _tex_id;

	gl_Position = proj * view * _model * vec4(in_pos.xy, 0.0f, 1.0f);
}
