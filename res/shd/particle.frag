#version 330

in vec2 pass_texcoord;
in vec4 pass_color;
flat in int  pass_render_mode;

uniform sampler2D tex;

out vec4 out_color;

void main(){
    if(pass_color.a == 0){
      discard;
    }

    if(pass_render_mode == 0){
        out_color = pass_color;
    }else{
        vec4 sample_c = texture(tex, pass_texcoord);

        if(sample_c.a == 0)
            discard;

        out_color = sample_c;
        out_color.a = pass_color.a;
    }
}
