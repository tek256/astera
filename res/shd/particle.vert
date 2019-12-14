#version 330

#define MAX_BATCH_SIZE 512

layout(location = 0) in vec4 in_pos;

uniform mat4 proj;
uniform mat4 view;

uniform vec2 tex_size;
uniform vec2 sub_size;

uniform int tex_ids[MAX_BATCH_SIZE];
uniform mat4 models[MAX_BATCH_SIZE];
uniform vec4 colors[MAX_BATCH_SIZE];

// 0 = color, 1 = textured
uniform int render_mode = 1;

out vec2 pass_texcoord;
out vec4 pass_color;
flat out int  pass_render_mode;

void main(){
  vec2 m_texc = in_pos.zw;
  if(render_mode == 1){
	  int _tex_id = tex_ids[gl_InstanceID];

  	int per_width = int(tex_size.x / sub_size.x);
	  vec2 offset = vec2(sub_size.x * (_tex_id % per_width), sub_size.y * (_tex_id / per_width));
  	m_texc = (offset / tex_size) + (sub_size / tex_size)  * m_texc;
  }

  pass_texcoord = m_texc; 
  pass_color = colors[gl_InstanceID];
  pass_render_mode = render_mode;

	gl_Position = proj * view * models[gl_InstanceID] * vec4(in_pos.xy, 1.0f, 1.0f); 
}
