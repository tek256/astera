#version 330

uniform sampler2d tex;

in int color_mode;
in vec3 _color;
in vec2 _texc;

out vec4 out_color;

void main(){
    if(color_mode == 0){
        out_color = vec4(_color, 1.f);
    }else if(color_mode == 1){
        out_color = texture(tex, _texc); 
    }else if(color_mode == 2){
        vec4 sample_color = texture(tex, _texc);
        out_color = sample_color * _color;
    }else{
        out_color = vec4(1, 0, 0, 1);
    }
}
