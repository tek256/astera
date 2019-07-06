#include "mem.h"
#include "debug.h"

#include <stdio.h>
#include <limits.h>

m_zone m_init_zone(size_t size){
	void* ptr = malloc(size);

	if(!ptr){
		_fatal("Unable to allocate memory zone of: %d\n", size);
		return (m_zone){0};
	}

	return (m_zone){ptr, NULL, size};
}

void m_free_zone(m_zone* zone){
#if defined(MEM_REUSE_ZONES)
	memset(zone->start, 0, zone->size + sizeof(m_tag));
	zone->start = NULL;
#else
	free(zone->start);
#endif	
}

//move to context for allocation & management 
void m_sw_ctx(m_zone* zone){
	ctx = zone;	
}

void m_update(m_zoneinfo* info){
	if(ctx){
		int blocks = 0;
		int free = 0;
		int used = 0;
		size_t free_amnt = 0;
		size_t used_amnt = 0;
		
		m_block* block = ctx->start;

		//free in front, thne the rest of that stuff
		//divide is the last free link in the chain
		m_block* divide = NULL;
		while(block){
			if(block->prev && block->next && !divide){
				if(block->prev->free != block->next->free){
					divide = block;
				}else{
					m_block* misnomer = (block->prev->free) ? block->prev : block->next;
					if(misnomer->prev){
						misnomer->prev->next = misnomer->next;
					}

					if(misnomer->next){
						misnomer->next->prev = misnomer->prev;
					}

					if(divide->prev){
						divide->prev->next = misnomer;
						divide->prev = misnomer;
					}	
				}
			}
			
			blocks++;
			if(block->free){
				free++;
				free_amnt += block->size;
			}else{
				used++;
				used_amnt += block->size;
			}

			if(block->next && block->next != ctx->start){
				block = block->next;
			}else{
				break;
			}
		}

		if(info){
			info->blocks = blocks;
			info->used = used;
			info->free = free;
			info->free_amnt = free_amnt;
			info->used_amnt = used_amnt;	
		}
	}else{
		_e("m_update called with no memory context to update.\n");
	}
}

void* m_alloc(size_t s){
	if(ctx){
		m_block* b = ctx->start;
		size_t real_size = s + sizeof(m_block) + sizeof(m_tag);
		
		m_block* last_suitable = NULL;
		size_t last_offset = LONG_MAX;

		while(b){
			if(b->free && b->size > real_size){
				size_t offset = b->size - real_size;
				if(offset < last_offset + MEM_OFFSET_THRESHHOLD){
					last_suitable = b;
					last_offset = offset;
				}
			}else if(b->free && b->size == real_size){
				last_suitable = b;
				last_offset = 0;
			}
			
			//break on last element
			if(b->next){
				b = b->next;
			}else{
				break;
			}
		}

		if(last_suitable){
			//split block
			if(last_suitable->size > real_size){
				void* new_offset = last_suitable->ptr + last_suitable->size + sizeof(m_tag);
				
				m_block* new_block = (m_block*)new_offset;
				new_block->ptr = new_offset + sizeof(m_block);
				new_block->size = last_suitable->size - (real_size + sizeof(m_tag) + sizeof(m_block));
				new_block->free = 1;
				m_tag* new_tag = (m_tag*)(new_block->ptr + new_block->size);
				m_tag* old_tag = (m_tag*)(new_offset - sizeof(m_tag));
				new_tag->size = new_block->size;
				new_tag->free = 1;
				
				old_tag->size = s;
				old_tag->free = 0;	

				last_suitable->free = 0;
				last_suitable->size = s;
				
				if(last_suitable->prev){
					last_suitable->prev->next = new_block;
				}

				if(last_suitable->next){
					last_suitable->next->prev = new_block;
				}

				b->next = last_suitable;
				last_suitable->prev = b;
				last_suitable->next = NULL;

				return last_suitable->ptr;
			}else{
				last_suitable->free = 0;
				m_tag* tag = (m_tag*)(last_suitable->ptr + last_suitable->size);
				tag->free = 0;

				if(last_suitable->prev){
					last_suitable->prev->next = last_suitable->next;
				}

				if(last_suitable->next){
					last_suitable->next->prev = last_suitable->prev;
				}
				
				b->next = last_suitable;
				last_suitable->prev = b;
				last_suitable->next = NULL;
					
				return last_suitable->ptr;	
			}
		}		
	}else{
		_e("No memory context selected to allocate to.\n");
		return NULL;
	}	
}

void* m_alloc_safe(size_t s, size_t* g){
	if(ctx){
		m_block* b = ctx->start;
		size_t real_size = s + sizeof(m_block) + sizeof(m_tag);
		
		m_block* last_suitable = NULL;
		size_t last_offset = LONG_MAX;

		while(b){
			if(b->free && b->size > real_size){
				size_t offset = b->size - real_size;
				if(offset < last_offset + MEM_OFFSET_THRESHHOLD){
					last_suitable = b;
					last_offset = offset;
				}
			}else if(b->free && b->size == real_size){
				last_suitable = b;
				last_offset = 0;
			}
			
			//break on last element
			if(b->next){
				b = b->next;
			}else{
				break;
			}
		}

		if(last_suitable){
			//split block
			if(last_suitable->size > real_size){
				void* new_offset = last_suitable->ptr + last_suitable->size + sizeof(m_tag);
				
				m_block* new_block = (m_block*)new_offset;
				new_block->ptr = new_offset + sizeof(m_block);
				new_block->size = last_suitable->size - (real_size + sizeof(m_tag) + sizeof(m_block));
				new_block->free = 1;
				m_tag* new_tag = (m_tag*)(new_block->ptr + new_block->size);
				m_tag* old_tag = (m_tag*)(new_offset - sizeof(m_tag));
				new_tag->size = new_block->size;
				new_tag->free = 1;
				
				old_tag->size = s;
				old_tag->free = 0;	

				last_suitable->free = 0;
				last_suitable->size = s;
				
				if(last_suitable->prev){
					last_suitable->prev->next = new_block;
				}

				if(last_suitable->next){
					last_suitable->next->prev = new_block;
				}

				b->next = last_suitable;
				last_suitable->prev = b;
				last_suitable->next = NULL;

				*g = last_suitable->size;
				return last_suitable->ptr;
			}else{
				last_suitable->free = 0;
				m_tag* tag = (m_tag*)(last_suitable->ptr + last_suitable->size);
				tag->free = 0;

				if(last_suitable->prev){
					last_suitable->prev->next = last_suitable->next;
				}

				if(last_suitable->next){
					last_suitable->next->prev = last_suitable->prev;
				}
				
				b->next = last_suitable;
				last_suitable->prev = b;
				last_suitable->next = NULL;
				
				*g = last_suitable->size;	
				return last_suitable->ptr;	
			}
		}		
	}else{
		_e("No memory context selected to allocate to.\n");
		*g = 0;
		return NULL;
	}	
}

m_block* m_free(void* ptr){
	m_block* b = ptr - sizeof(m_block);
	m_tag* t = (ptr + b->size);
	b->free = 1;
	t->free = 1;
#if defined(MEM_SAFE)
	memset(ptr, 0, b->size);
#endif

	//join freed blocks
	m_tag* p = ptr - sizeof(m_block) - sizeof(m_tag);
	if(p->free){
		m_block* prev = p - p->size - sizeof(m_block);
		prev->size += sizeof(m_block) + sizeof(m_tag);
		prev->size += b->size;
		memset(p, 0, sizeof(m_block) + sizeof(m_tag));
		t->size = prev->size;			
	}
}


