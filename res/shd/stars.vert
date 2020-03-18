#version 330

#define MAX_PARTICLES 512

layout(location = 0) in vec4 in_vert;

uniform mat4 models[MAX_PARTICLES];
uniform vec4 colors[MAX_PARTICLES];

uniform mat4 proj;
uniform mat4 view;

out vec2 pass_tex;
out vec4 pass_color;

void main(){
  pass_tex = in_vert.zw;
  pass_color = colors[gl_InstanceID];

  gl_Position = proj * view * models[gl_InstanceID] * vec4(in_vert.xy, 1.0, 1.0);
}

