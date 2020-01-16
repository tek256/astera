#ifndef ASSET_H
#define ASSET_H

//#define ASSET_NO_SYS_MAP
#include <stdint.h>

#include "platform.h"

// Asset Configurations
#if !defined(MAX_ASSET_CACHE)
#define MAX_ASSET_CACHE 256
#endif

#if !defined(MIN_ASSET_CACHE)
#define MIN_ASSET_CACHE 16
#endif

#if !defined(ASSET_CACHE_GROWTH)
#define ASSET_CACHE_GROWTH 16
#endif

// Max amount of map_t's
#if !defined(ASSET_MAX_MAPS)
#define ASSET_MAX_MAPS 4
#endif

typedef struct {
  unsigned int uid;
  unsigned char *data;
  unsigned int data_length;
  const char *name;
  int filled : 1;
  int req : 1;
  int req_free : 1;
} asset_t;

typedef struct {
  asset_t *assets;
  unsigned int count;
  unsigned int capacity;
  const char *name;
  const char *filename;
  uint8_t compression_level;
  int free : 1;
} asset_map_t;

static asset_map_t asset_maps[ASSET_MAX_MAPS];
static unsigned int asset_map_count = 0;
static unsigned int asset_uid_count = 0;

int asset_init();
void asset_exit();

asset_t *asset_get(const char *map_name, const char *file);
asset_t *asset_req(const char *map_name, const char *file);

asset_map_t *asset_create_map(const char *filename, const char *name,
                              unsigned int capacity, uint8_t compression_level);

void asset_update_map(asset_map_t *map);

void asset_free(asset_t *asset);
void asset_map_free(const char *map);
void asset_map_free_t(asset_map_t *map);

#endif
