#include "game.h"
#include "audio.h"
#include "debug.h"
#include "render.h"
#include "input.h"

static r_camera camera;
static a_buf buffer;
static r_shader shader;
static r_sheet sheet;
static r_tex tex;
static r_drawable drawable;
static r_anim anim;
static r_animv animv;

static r_leaf tree;

int g_init(){
	r_create_camera(&camera, (v2){160.f, 90.f}, (v2){0.f, 0.f});
	tex = r_get_tex("res/tex/test_sheet.png");
	sheet = r_create_sheet(&tex, 16, 16);
	shader = r_create_shader("res/shd/main.v", "res/shd/main.f");

	anim = (r_anim){ &sheet, 4, {1, 2, 3, 4}, 24 };
   	animv = r_create_animv(&anim);	
	
	drawable = (r_drawable){
		&animv, 
		(v2){ 128.f, 128.f },
		(v3){ 0.f, 0.f, 0.f },
		(m4){0},
		1
	};

	tree = r_create_leaf(R_LEAF_SHADER, &shader, NULL, NULL);	
	
	r_ins_tex(&tree, &sheet);
	r_ins_drawable(&tree, &shader, &drawable);

	return 1;	
}

void g_exit(){
	_l("Exiting game.");
}

void g_input(long delta){
	if(i_key_down('A')){
		_l("oh...\n");
	}

	if(i_key_clicked('P')){
		a_play_sfx(&buffer, 1.f, (v2){0.f, 0.f}); 
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){	
	a_update(delta);
	r_update(delta);
}

void g_render(long delta){
	r_update_camera(&camera);

	r_leaf* shader_curs = &tree;
	while(shader_curs){
		r_shader* shader = (r_shader*)shader_curs->value;

		if(shader){
			r_bind_shader(shader_curs->value);

			r_set_m4(shader_curs->value, "proj", camera.proj);
			r_set_m4(shader_curs->value, "view", camera.view);

			r_leaf* tex_curs = shader_curs->child;
			while(tex_curs){
				r_bind_tex(((r_sheet*)tex_curs->value)->tex);
				r_sheet* sheet = (r_sheet*)tex_curs->value;

				v2 tex_size = (v2){
					sheet->tex->width,
						sheet->tex->height	
				};

				v2 sub_size = (v2){
					sheet->subwidth,
						sheet->subheight
				};

				r_set_v2(shader_curs->value, "tex_size", tex_size);
				r_set_v2(shader_curs->value, "sub_size", sub_size);

				int draw_count = 0;

				r_leaf* draw_curs = tex_curs->child;
				while(draw_curs){
					r_drawable* draw = (r_drawable*)draw_curs->value;
					int frame_id = draw->anim->anim->frames[draw->anim->frame];

					if(draw->visible){
						mats[draw_count] = draw->model;
						tex_ids[draw_count] = frame_id;
						draw_count ++;
					}

					if(draw_count == RENDER_BATCH_SIZE){
						r_draw_call(shader_curs->value, draw_count);
						memset(mats, 0, sizeof(m4) * RENDER_BATCH_SIZE);
						memset(tex_ids, 0, sizeof(int) * RENDER_BATCH_SIZE);
						draw_count = 0;
					}
					if(draw_curs->next){
						draw_curs = draw_curs->next;
					}else{
						break;
					}
				}

				if(draw_count > 0){
					r_draw_call(shader_curs->value, draw_count);
					memset(mats, 0, sizeof(m4) * RENDER_BATCH_SIZE);
					memset(tex_ids, 0, sizeof(int) * RENDER_BATCH_SIZE);
					draw_count = 0;
				}	
				if(tex_curs->next){
					tex_curs = tex_curs->next;
				}else{
					break;
				}
			}

		}
		if(shader_curs->next){
			shader_curs = shader_curs->next;
		}else{
			break;
		}
	}
}
