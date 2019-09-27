#include "asset.h"

#include <string.h>

#define REQ_STACK_START_SIZE 8
#define REQ_STACK_GROWTH 8

asset_t* asset_to_map(asset_t asset, asset_map_t* map){
	if(!map){
		_e("No map passed to add to.\n");
		return 0;
	}

	for(int i=0;i<map->count;++i){
		if(strcmp(map->assets[i].name, asset.name) == 0){
			_e("%s is already in this map.\n", asset.name);
			return 0;
		}	
	}

	if(!map->capacity || !map->assets){
		map->assets = (asset_t*)malloc(sizeof(asset_t) * MIN_ASSET_CACHE);
		map->capacity = MIN_ASSET_CACHE;			

		if(!map->assets){
			_e("Unable to allocate initial array for map [%i] bytes.\n", sizeof(asset_t) * MIN_ASSET_CACHE);
			return 0;
		}
	}

	if(map->capacity == map->count){
		int new_size = sizeof(asset_t) * (map->capacity + ASSET_CACHE_GROWTH);
		asset_t* new_array = (asset_t*)malloc(new_size);
		
		if(!new_array){
			_e("Unable to allocate space for: %i bytes.\n", new_size);
		}
		
		memcpy(new_array, map->assets, sizeof(asset_t) * map->capacity);
		free(map->assets);
		map->capacity += ASSET_CACHE_GROWTH;
		map->assets = new_array;	
	}
	
	map->assets[map->count] = asset;
	++map->count;
	return &map->assets[map->count-1];	
}

asset_t load_asset(const char* archive, const char* filename){
	struct zip_t* zip = zip_open(archive, 0, 'r');
	zip_entry_open(zip, filename);

	unsigned int file_size = zip_entry_size(zip);
	unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * file_size);

	zip_entry_noallocread(zip, (void*)data, file_size);
	zip_entry_close(zip);
	zip_close(zip);

	return (asset_t){0, data, file_size, filename};
}

asset_req_t* asset_request(const char* archive, const char* filename, asset_map_t* map_to){
	asset_req_t req;

	if(!archive || !filename){
		_e("Invalid request.\n");
		return 0;
	}

	req.name = filename;
	req.map_to = map_to;

	if(req_stack.archive){
		if(strcmp(archive, req_stack.archive) == 0){
			if(req_stack.capacity == 0){
				req_stack.capacity = REQ_STACK_START_SIZE; 
				req_stack.reqs = (asset_req_t*)malloc(sizeof(asset_req_t) * req_stack.capacity);
			}else if(req_stack.capacity == req_stack.count){
				asset_req_t* reqs = (asset_req_t*)malloc(sizeof(asset_req_t) * (req_stack.capacity + REQ_STACK_GROWTH));
				memcpy(reqs, req_stack.reqs, sizeof(asset_req_t) * req_stack.count);	
				free(req_stack.reqs);
				req_stack.capacity += REQ_STACK_GROWTH;
			}			

			req_stack.reqs[req_stack.count] = req;
			++req_stack.count;
			return &req_stack.reqs[req_stack.count-1];
		}	
	}else{
		if(req_stack.capacity == 0){
			req_stack.capacity = REQ_STACK_START_SIZE; 
			req_stack.reqs = (asset_req_t*)malloc(sizeof(asset_req_t) * req_stack.capacity);
		}else if(req_stack.capacity == req_stack.count){
			asset_req_t* reqs = (asset_req_t*)malloc(sizeof(asset_req_t) * (req_stack.capacity + REQ_STACK_GROWTH));
			memcpy(reqs, req_stack.reqs, sizeof(asset_req_t) * req_stack.count);	
			free(req_stack.reqs);
			req_stack.capacity += REQ_STACK_GROWTH;
		}

		req_stack.archive = archive;
		req_stack.reqs[req_stack.count] = req;
		++req_stack.count;
		return &req_stack.reqs[req_stack.count-1];
	}
}

void asset_pop_stack(){
	struct zip_t* zip = zip_open(req_stack.archive, 0, 'r');

	for(int i=0;i<req_stack.count;++i){
		asset_req_t* req = &req_stack.reqs[i];
		if(!req->name){
			req->data = NULL;
			req->length = 0;
			req->filled = 0;
			zip_entry_close(zip);
			continue;
		}

		if(zip_entry_open(zip, req->name)){
			req->length = zip_entry_size(zip);
			req->data = (unsigned char*)malloc(sizeof(char) * req->length);
			if(!req->data){
				_e("Unable to allocate %i bytes for zip entry %s.\n", req->length, req->name);
				req->data = 0;
				zip_entry_close(zip);
				zip_close(zip);
				break;
			}

			if(!zip_entry_noallocread(zip, (void*)req->data, req->length)){
				_e("Unable to read %s from zip.\n", req->name);
				zip_entry_close(zip);
				continue;
			}

			req->filled = 1;		
		}else{
			_l("Entry %s not found.\n", req->name);
			continue;
		}

		zip_entry_close(zip);
	}

	zip_close(zip);
}

void asset_clear_stack(){
	memset(req_stack.reqs, 0, sizeof(asset_req_t) * req_stack.count);
	req_stack.count = 0;
}

asset_map_t* asset_create_map(const char* name, unsigned int capacity){
	if(asset_map_count == ASSET_MAX_MAPS){
		_e("Unable to create map, at max maps.\n");
		return 0;
	}	

	int index = -1;
	for(int i=0;i<ASSET_MAX_MAPS;++i){
		if(!asset_maps[i]->name){
			index = i;
			break;
		}		
	}

	asset_map_t* map = &asset_maps[index];
	asset_t* cache = (asset_t*)malloc(sizeof(asset_t) * capacity);
	if(!cache){
		_e("Unable to allocate space for cache with [%i] bytes.\n", sizeof(asset_t) * capacity);
		return 0;
	}

	map->name = name;
	
	return map;	
}

void asset_destroy_cache(asset_map_t* map){
	free(map->assets);

	map->capacity = 0;
	map->assets = 0;
	map->count = 0;

	asset_map_count --;
}


