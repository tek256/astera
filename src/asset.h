#ifndef ASSET_H
#define ASSET_H

#include <stdio.h>
#include <zip/zip.h>

#include "config.h"
#include "platform.h"

typedef struct {
	unsigned int uid;
	unsigned char* data; 
	unsigned int data_length;
	const char* name;
	int filled : 1;
	int req : 1;
	int req_free : 1;	
} asset_t;

typedef struct {
	asset_t* assets;
	unsigned int count;	
	unsigned int capacity;
	const char* name;
	const char* filename; 
	int free : 1;
} asset_map_t;

static asset_map_t asset_maps[ASSET_MAX_MAPS];
static unsigned int asset_map_count = 0;
static unsigned int asset_uid_count = 0;

int asset_init();
void asset_exit();

asset_t* asset_get(const char* map_name, const char* file);
asset_t* asset_req(const char* map_name, const char* file);

asset_map_t* asset_create_map(const char* filename, const char* name, unsigned int capacity);

void asset_update_map(asset_map_t* map);

void asset_free(asset_t* asset);
void asset_map_free(const char* map);

#endif
