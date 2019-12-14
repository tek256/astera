#version 330

in vec2 pass_texcoord;
flat in int pass_texid;

uniform int c_mode = -1;
uniform sampler2D tex;
uniform vec4 c;

out vec4 out_c;

void main(){
    if(c_mode == 1){
      if(c.a == 0)
        discard;

      out_c = c;
    }else{
        vec4 sample_c = texture(tex, pass_texcoord);

        if(sample_c.a == 0)
            discard;

        if(c_mode == -1){
            out_c = sample_c;
        }else{
            out_c = sample_c * c;
        }
    }
}
