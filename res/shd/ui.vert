#version 330

#define MAX_PASS 128

layout(location = 0) in vec4 vert;

uniform mat4 proj;

uniform vec2 tex_size; 

uniform int draw_mode = 0; // 0 = standard, 1 = text

uniform vec4 texcoords[MAX_PASS];
uniform vec4 posoffsets[MAX_PASS];
uniform vec3 colors[MAX_PASS];

out int color_mode;
out vec3 _color;
out vec2 _texc;

void main(){
	vec2 tex_coord = vert.zw;
	vec4 pass_coord = texcoords[gl_InstanceID];
	vec3 color = colors[gl_InstanceID];

	if(draw_mode == 0){
		vec2 sub_size = texcoords[gl_InstanceID].xy;

		int per_width = sub_size.x / tex_size.x;
		int tex_id = (int)texcoords[gl_InstanceID].z; 
		int sub_x = tex_id % per_width;
		int sub_y = tex_id / per_width;
		vec2 _texcoord = (vec2(sub_x * sub_size.x, sub_y * sub_size.y) / tex_size) * tex_coord;

		_texc = _texcoord;
	}else{
		//TODO refactor into less operations
		vec2 _sub_size = vec2(pass_coord.z - pass_coord.x, pass_coord.w - pass_coord.y);
		vec2 _sub_offfset = vec2(pass_coord.x, pass_coord.y);
		vec2 _texcoord = ((_sub_size / tex_size) + (_sub_offset / tex_size)) * tex_coord;
		
		_texc = _texcoord;	
	}

	_color = color;
	
	mat4 model;
	translate(model, vec4(posoffsets[gl_InstanceID].xy, 0.0, 1.0));
	scale(model, vec4(posoffsets[gl_InstanceID].zw, 1.0, 1.0));

	gl_Position = proj * model * vec4(vert.xy, 1.0);
}
