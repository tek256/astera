#include "game.h"
#include "debug.h"
#include "render.h"
#include "input.h"

static r_list* draw_tree;
static r_camera camera;

void g_init(){
	_l("Starting game.");
	r_create_camera(&camera, (v2){160.0, 90.0f}, (v2){0.f});
	r_shader shader = r_create_shader("res/shd/main.v", "res/shd/main.f");
	r_tex tex = r_get_tex("res/tex/test_sheet.png");
	r_sheet sheet = r_create_sheet(&tex, 16, 16);
	r_anim anim = (r_anim){&sheet, 4, {0, 1, 2, 3}, 24};
	*draw_tree = r_create_list(&shader, &sheet);

	r_animv v = r_create_animv(&anim);
	r_drawable drawable = (r_drawable){
		&v,	//animv
		(v2){16.f, 16.f}, //size
	   	(v3){0.f}, //pos
	   	(m4){0.f}, //model 
	   	1 //visible
	};	
}

void g_exit(){
	_l("Exiting game.");
}

void g_input(long delta){
	if(i_key_down('A')){
		_l("oh.\n");
	}

	if(i_key_clicked(GLFW_KEY_ESCAPE)){
		r_request_close();	
	}
}

void g_update(long delta){	
	
}

static int u_tex_size, u_sub_size, u_tex_id, u_got;
static int u_proj, u_view, u_model;
void g_render(long delta){
	r_sleaf* c = draw_tree->root;
	while(c){
		r_bind_shader(c->val);

		if(!u_got){
			u_tex_size = r_get_uniform_loc(c->val, "tex_size");
			u_sub_size = r_get_uniform_loc(c->val, "sub_size");
			u_proj = r_get_uniform_loc(c->val, "proj");
			u_view = r_get_uniform_loc(c->val, "view");
			u_got = 1;
		}

		r_set_m4i(u_view, camera.view);
		r_set_m4i(u_proj, camera.proj);

		r_tleaf* tex_leaf = c->leafs;
		while(tex_leaf){
			r_bind_tex(tex_leaf->val->tex);

			v2 tex_size = (v2){
				tex_leaf->val->tex->width,
				tex_leaf->val->tex->height
			};

			v2 sub_size = (v2){
				tex_leaf->val->subwidth,
				tex_leaf->val->subheight
			};

			r_set_v2i(u_tex_size, tex_size);
			r_set_v2i(u_sub_size, sub_size);

			r_leaf* leaf = tex_leaf->leafs;
			int sprite_count = 0;
			while(leaf){
				if(leaf->val->visible){
					int f_index = leaf->val->anim->frame;
					int f_value = leaf->val->anim->anim->frames[f_index];
					m4 model = leaf->val->model;
					
					if(sprite_count == RENDER_BATCH_SIZE){
						r_draw_call(c->val, sprite_count);
						sprite_count = 0;
						memset(tex_ids, 0, RENDER_BATCH_SIZE * sizeof(int));
						memset(mats, 0, RENDER_BATCH_SIZE * sizeof(m4));
					} else {
						mats[sprite_count] = model;
						tex_ids[sprite_count] = f_value;
						sprite_count ++;
					}
				}

				if(leaf->next != tex_leaf->leafs){
					leaf = leaf->next;
				}else{
					r_draw_call(c->val, sprite_count);
					sprite_count = 0;
					memset(tex_ids, 0, RENDER_BATCH_SIZE * sizeof(int));
					memset(mats, 0, RENDER_BATCH_SIZE * sizeof(m4));
					break;
				}
			}

			if(tex_leaf->next != c->leafs){
				tex_leaf = tex_leaf->next;
			}
		}

		if(c->next != draw_tree->root){
			c = c->next;
		}
	}
}
