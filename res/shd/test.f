#version 330

in vec2 out_tc;
uniform sampler2D tex;

out vec4 out_c;

void main(){
    vec4 color = texture(tex, out_tc); 
    out_c = vec4(1, 0, 0, 1);
    //out_c = color; 
}
