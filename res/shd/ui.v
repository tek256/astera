#version 330

layout(location = 0) in vec4 vert;

uniform float unit_size;

uniform int sub_tex = -1;

uniform vec2 tex_size;
uniform vec2 sub_size;

uniform vec2 offset;
uniform vec2 scale;
uniform mat4 proj;

out vec2 tex_coord;

void main(){
	vec2 tex_c = vert.zw;

	if(sub_tex != -1){
		int per_width = int(tex_size.x / sub_size.x);
		vec2 _offset = vec2(sub_size.x * (_tex_id % per_width), sub_size.y * (_tex_id / per_width));
	
		tex_c = (_offset / tex_size) + (sub_size / tex_size)  * tex_c;

		tex_coord = tex_c;
	}

	vec3 pos = vec3(offset, 1) + vec3(vert.xy, 1);
	pos *= vec3(scale, 1.0);
	
	//TODO pixel clamping	
	vec4 out_position = proj * vec4(pos.x, pos.y, 1.0);
	gl_Postition = out_position;
}
