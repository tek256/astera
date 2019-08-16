#version 330

in vec2 tex_coord;
uniform sampler2d tex;

//0 = color, 1 = texture
uniform int mode = 0;
uniform vec4 color;

out vec4 out_color;

void main(){
   if(mode == 1){
       vec4 sample_color = texture(tex, tex_coord);

       if(color.a == 0) discard;

       out_color = sample_color;
   }else if(mode == 0){
       if(color.a == 0) discard;
       out_color = color; 
   }else{
       out_color = vec4(1, 0, 0, 1); 
   }
}
