#include "asset.h"
#include "debug.h"

#include <string.h>

int asset_init(){
	for(int i=0;i<ASSET_MAX_MAPS;++i){
		asset_maps[i].free = 1;
	}

	asset_create_map("res.zip", "default", MIN_ASSET_CACHE);
	asset_create_map(NULL, "sys", MIN_ASSET_CACHE);
	return 1;
}

void asset_exit(){
	for(int i=0;i<ASSET_MAX_MAPS;++i){
		asset_map_free(&asset_maps[i]);
	}
}

void asset_free(asset_t* asset){
	asset->name = 0;
	asset->filled = 0;
	asset->req = 0;
	free(asset->data);
}

void asset_map_free(const char* map_name){
	asset_map_t* map;
	for(unsigned int i=0;i<asset_map_count;++i){
		if(strcmp(asset_maps[i].name, map_name) == 0){
			map = &asset_maps[i];
			break;
		}else if(strcmp(asset_maps[i].filename, map_name) == 0){
			map = &asset_maps[i];
			break;
		}
	}

	if(!map) {
		_e("Unable to find map %s to free.\n", map);
		return;
	}

	for(unsigned int i=0;i<map->capacity;++i){
		asset_t* asset = &map->assets[i];
		if(asset->data){
			free(asset->data);
			asset->data = 0;
		}	
	}

	free(map->assets);
	map->assets = 0;
	map->count = 0;
	map->capacity = 0;
	map->name = 0;
	map->filename = 0;
}

asset_t* asset_get(const char* map_name, const char* file){	
	asset_map_t* map = NULL;
	if(!map_name){
		map_name = "sys";
	}

	for(int i=0;i<ASSET_MAX_MAPS;++i){
		if(map_name && asset_maps[i].name){
			if(strcmp(asset_maps[i].name, map_name) == 0){
				map = &asset_maps[i];	
				break;
			}
		}else if(asset_maps[i].filename && map_name){
			if(strcmp(asset_maps[i].filename, map_name) == 0){
				map = &asset_maps[i];
				break;
			}
		}
	}

	if(!map) {
		_l("No map with name or filepath of: %s\n", map_name);
		return 0;
	}

	asset_t* asset = NULL;
	int free = 0;
	for(unsigned int i=0;i<map->capacity;++i){
		if(!map->assets[i].filled && !map->assets[i].req){
			++free;
			continue;
		}else if(map->assets[i].filled){
			continue;
		}

		if(strcmp(map->assets[i].name, file) == 0){
			if(!map->assets[i].req) map->assets[i].req = 1;
			return &map->assets[i];	
		}
	}
	
	//At capacity for asset map
	if(!free) return 0;

	for(unsigned int i=0;i<map->capacity;++i){
		if(!map->assets[i].filled && !map->assets[i].req){
			asset = &map->assets[i];
			break;			
		}
	}

	if(strcmp(map->name, "sys") == 0){
		FILE* f = fopen(file, "r+");
		
		if(!f){
			_e("Unable to open system file: %s\n", file);
			return NULL;
		}

		fseek(f, 0, SEEK_END);
		unsigned int file_size = ftell(f);
		rewind(f);

		unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * file_size);

		if(!data){
			_e("Unable to allocate %i bytes for file %s\n", file_size, file);
			fclose(f);
			return NULL;
		}

		unsigned int data_read = fread(data, sizeof(unsigned char), file_size, f);

		if(data_read != file_size){
			_l("Incomplete read: %i expeceted, %i read.\n", file_size, data_read);
		}	
	
		fclose(f);
		asset->name = file;
		asset->data = data;
		asset->data_length = data_read;
		asset->filled = 1;
		asset->req = 0;
		asset->req_free = 0;
	}else{
		struct zip_t* zip;
		
		zip = zip_open(map->filename, COMPRESSION_LEVEL, 'r');
		if(!zip){
			_e("Unable to open zip entry: %s.\n", map->filename);
			return NULL;
		}

		if(zip_entry_open(zip, file) < 0){
			_e("Unable to open file: %s in zip archive: %s\n", file, map->filename);
			zip_close(zip);
			return NULL;
		}

		unsigned int entry_size = zip_entry_size(zip);
		unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char) * entry_size);
		if(!data){
			_e("Unable to allocate %i bytes for file %s.\n", entry_size, file);
			zip_close(zip);
			return NULL;
		}

		zip_entry_noallocread(zip, (void*)data, entry_size);
			
		zip_close(zip);

		asset->name = file;
		asset->data = data;
		asset->data_length = entry_size;
		asset->filled = 1;
		asset->req = 0;
		asset->req_free = 0;
	}

	asset->uid = ++asset_uid_count;

	return asset;

}

asset_t* asset_req(const char* map_name, const char* file){
	asset_map_t* map;
	for(int i=0;i<asset_map_count;++i){
		if(strcmp(asset_maps[i].name, map_name) == 0){
			map = &asset_maps[i];	
			break;
		}else if(strcmp(asset_maps[i].filename, map_name) == 0){
			map = &asset_maps[i];
			break;
		}
	}

	if(!map) {
		_l("No map with name or filepath of: %s\n", map_name);
		return 0;
	}

	asset_t* asset;
	int free = 0;
	for(unsigned int i=0;i<map->capacity;++i){
		if(!map->assets[i].filled && !map->assets[i].req){
			++free;
			continue;
		}else if(map->assets[i].filled){
			continue;
		}

		if(strcmp(map->assets[i].name, file) == 0){
			if(!map->assets[i].req) map->assets[i].req = 1;
			return &map->assets[i];	
		}
	}
	
	//At capacity for asset map
	if(!free) return 0;

	for(unsigned int i=0;i<map->capacity;++i){
		if(!map->assets[i].filled && !map->assets[i].req){
			asset = &map->assets[i];
			break;			
		}
	}

	asset->req = 1;
	asset->filled = 0;
	asset->name = file;	
	asset->uid = ++asset_uid_count;

	return asset;
}

asset_map_t* asset_create_map(const char* filename, const char* name, unsigned int capacity){
	int free_maps = 0;
	for(int i=0;i<ASSET_MAX_MAPS;++i){
		if(asset_maps[i].free){
			++free_maps;
			continue;
		}
		
		if(filename){
			if(strcmp(filename, asset_maps[i].filename) == 0){
				return &asset_maps[i];
			}
		}
		
		if(strcmp(name, asset_maps[i].name) == 0){
			return &asset_maps[i];
		}
	}
	
	if(!free_maps){
		return 0;
	}

	for(int i=0;i<ASSET_MAX_MAPS;++i){
		if(asset_maps[i].free){
			asset_maps[i].assets = (asset_t*)malloc(sizeof(asset_t) * capacity);
			asset_maps[i].capacity = capacity;
			asset_maps[i].filename = filename;
			asset_maps[i].name = name;
			asset_maps[i].free = 0;
			return &asset_maps[i];			
		}
	}

	return 0;	
}

void asset_update_map(asset_map_t* map){
	struct zip_t* zip;

	for(unsigned int i=0;i<map->capacity;++i){
		if(map->assets[i].req_free){
			free(map->assets[i].data);
			memset(&map->assets[i], 0, sizeof(asset_t));
			continue;
		}

		if(map->assets[i].req){
			if(!zip) zip = zip_open(map->filename, COMPRESSION_LEVEL, 'r');
			if(!zip) {
				_e("Unable to open: %s\n", map->filename);
			   	break;
			}

			if(zip_entry_open(zip, map->assets[i].name) < 0){
				_e("Unable to open file: %s in archive: %s\n", map->assets[i].name, map->filename);
				zip_entry_close(zip);
				continue;
			}

			int file_size = zip_entry_size(zip);
			if(!file_size){
				_e("Unable to open file with size of 0, %s.\n", map->assets[i].name);
				continue;
			}

			map->assets[i].data = (unsigned char*)malloc(file_size * sizeof(unsigned char));
			map->assets[i].data_length = file_size;	
			map->assets[i].req = 0;
			map->assets[i].filled = 1;

			zip_entry_noallocread(zip, (void*)map->assets[i].data, file_size);
			zip_entry_close(zip);
			++map->count;
		}	
	}

	if(zip) zip_close(zip);
}
