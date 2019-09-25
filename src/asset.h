#ifndef ASSET_H
#define ASSET_H

#include <stdio.h>

#include <zip/zip.h>

#include "platform.h"

#define MAX_ASSET_CACHE 256
#define MIN_ASSET_CACHE 16
#define ASSET_CACHE_GROWTH 16

//Max amount of map_t's
#define ASSET_MAX_MAPS 4

typedef struct {
	unsigned int uid;
	unsigned char* data; 
	unsigned int data_length;
	const char* name;	
} asset_t;

typedef struct {
	asset_t* assets;
	unsigned int asset_count;	
} asset_map_t;

typedef struct {
	asset_t** to_save;	
	u16 save_count;
	u16 save_capacity;
} asset_flags;

//leave these as pointers for when they're allocated
static asset_map_t* asset_maps[ASSET_MAX_MAPS];
static int asset_map_count = 0;

static asset_flags flags;

asset_t load_asset(const char* archive, const char* filename);

void asset_save(asset_t* a);
asset_map_t* asset_load_cache(const char* filename, const char* section);
void asset_destroy_cache(asset_map_t* map); 

#endif
