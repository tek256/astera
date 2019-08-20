#include "phys.h"

#include <linmath.h>

static p_map _map;
/* TODO implementation of this
void p_init(vec2 size, vec2 block_size){
	int _x = size.x / block_size.x;
	int _y = size.y / block_size.y;

}

void p_load(g_entity* entities, unsigned int count){

}

void p_swap(g_entity* entities, unsigned int count){

}

void p_update(long delta){

}

vec2 p_get_pos(p_entity* entity){
	return entity->col.col.pos;
}

vec2 p_get_vel(p_entity* entity){
	return entity->vel;
}

void p_selective_update(p_block* block, long delta){
	vec2 _delta = (vec2){delta, delta};
	vec2 potential;

	for(int i=0;i<block->count;++i){
		if(block->entites[i]->active){
			vec2_add(block->entities[i]->col.col.pos, vec2_mul(block->entities[i]->vel, _delta));	
		}
	}

	for(int i=0;i<block->count;++i){
		for(int j=0;j<block->count;++j){
			if(p_interact(potential, block->entities[i], block->entities[j])){
				p_resolve(potential, block->entities[i], block->entities[j]);
			}
		}
	}
}

int p_interact(vec2 _potential, p_entity* a, p_entity* b){
	vec2 solution;
	switch(a->col.type){
		case P_CIRCLE:
			switch(b->col.type){
				case P_CIRCLE:
					p_circle_circle(solution, a->col.col, b->col.col);
					break;
				case P_AABB:
					p_circle_aabb(solution, a->col.col, b->col.col);
					break;
				case P_PLANE:
					p_circle_plane(solution, a->col.col, b->col.col;
					break;
				case P_BOX:
					p_circle_box(solution, a->col.col, b->col.col);
					break;
			}
			break;
		case P_AABB:
			switch(b->col.type){
				case P_CIRCLE:
					p_circle_aabb(solution, b->col.col, a->col.col);	
					break;
				case P_AABB:
					p_aabb_aabb(solution, a->col.col, b->col.col);
					break;
				case P_PLANE:
					p_aabb_plane(solution, a->col.col, b->col.col;
					break;
				case P_BOX:
					p_aabb_box(solution, a->col.col, b->col.col);
					break;
			}
			break;
		case P_PLANE:
			switch(b->col.type){
				case P_CIRCLE:
					p_circle_plane(solution, b->col.col, a->col.col);
					break;
				case P_AABB:
					p_aabb_plane(solution, b->col.col, a->col.col);
					break;
				case P_PLANE:
					p_plane_plane(solution, a->col.col, b->col.col);
					break;
				case P_BOX:
					p_box_plane(solution, b->col.col, a->col.col);
					break;
			}
			break;
		case P_BOX:
			switch(b->col.type){
				case P_CIRCLE:
					p_circle_box(solution, b->col.col, a->col.col);
					break;
				case P_AABB:
					p_aabb_box(solution, b->col.col, a->col.col);
					break;
				case P_PLANE:
					p_box_plane(solution, a->col.col, b->col.col);
					break;
				case P_BOX:
					p_box_box(solution, a->col.col, b->col.col);
					break;
			}
			break;
	}

	if(solution.x != 0 || solution.y != 0){
		vec2_dup(_potential, solution);
		return 1;
	}

	return 0;
}

void p_resolve(vec2 potential, p_entity* a, p_entity* b){
	
}

void p_exit(){

}

//TODO: Math
void p_circle_aabb(vec2 sol, circ* _ci, aabb* _aa){

}

void p_circle_plane(vec2 sol, circ* _ci, plane* _pl){

}

void p_circle_box(vec2 sol, circ* _ci, box* _b){

}

void p_circle_circle(vec2 sol, circ* _ci, circ* _ci2){

}

void p_box_aabb(vec2 sol, box* _b, aabb* _aa){

}

void p_box_plane(vec2 sol, box* _b, plane* _pl){

}

void p_box_box(vec2 sol, box* _b, box* _b2){

}

void p_aabb_aabb(vec2 sol, aabb* _aa, aabb* _bb){

}

void p_aabb_plane(vec2 sol, aabb* _aa, plane* _pl){

}

void p_plane_plane(vec2 sol, plane* _pl, plane* _pl2){

}
*/
