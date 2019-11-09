#version 330

//I should document these as well.

in vec2 o_texc;

uniform int c_mode = -1;
uniform sampler2D tex;
uniform vec4 c;

out vec4 out_c;

void main(){
    if(c_mode == 1){
        out_c = c;
    }else{
        vec4 sample_c = texture(tex, o_texc);

        if(sample_c.a == 0)
            discard;

        if(c_mode == -1){
            out_c = sample_c;
        }else{
            out_c = sample_c * c;
        }
    }
}
