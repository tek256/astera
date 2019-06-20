#include "game.h"
#include "debug.h"
#include "render.h"
#include "input.h"

static r_list* drawables;
static r_camera camera;

void g_init(){
	_l("Starting game.");
	r_create_camera(&camera, (v2){160.0, 90.0f}, (v2){0.f});
	r_shader shader = r_create_shader("res/shd/main.v", "res/shd/main.f");
	r_tex tex = r_get_tex("res/tex/test.png");
	r_sheet sheet = r_create_sheet(&tex, 16, 16);
	*drawables = r_create_list(&shader, &sheet);
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
	r_sleaf* c = drawables->root;
	while(c){
		r_bind_shader(c->val);

		if(!u_got){
			u_tex_size = r_get_uniform_loc(c->val, "tex_size");
			u_sub_size = r_get_uniform_loc(c->val, "sub_size");
			u_tex_id = r_get_uniform_loc(c->val, "tex_id");
			u_proj = r_get_uniform_loc(c->val, "proj");
			u_view = r_get_uniform_loc(c->val, "view");
			u_model = r_get_uniform_loc(c->val, "model");
			u_got = 1;
		}

		r_set_m4i(u_view, camera.view);
		r_set_m4i(u_proj, camera.proj);

		r_tleaf* tex_leaf = c->texs;
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
			while(leaf){
				r_set_uniformii(u_tex_id, leaf->val->anim->frame);
				r_set_m4i(u_model, leaf->val->model);

				r_draw_def_quad();

				if(leaf->next != tex_leaf->leafs){
					leaf = leaf->next;
				}
			}

			if(tex_leaf->next != c->texs){
				tex_leaf = tex_leaf->next;
			}
		}

		if(c->next != drawables->root){
			c = c->next;
		}
	}
}
