#ifndef ASTERA_ASSET_HEADER
#define ASTERA_ASSET_HEADER

#ifdef __cplusplus
extern "C" {
#endif

//#define ASSET_NO_SYS_MAP -- this one is a tricky one, it'll break _everything_
// but I don't know that I want to have to require system map

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

  int32_t chunk_start, chunk_length;

  int filled : 1;
  int req : 1;
  int req_free : 1;
  int chunk : 1;
} asset_t;

typedef struct {
  asset_t *assets;
  uint32_t count;
  uint32_t capacity;
  const char *name;
  const char *filename;
  uint8_t compression_level;
  int free : 1;
} asset_map_t;

static asset_map_t asset_maps[ASSET_MAX_MAPS];
static unsigned int asset_map_count = 0;
static unsigned int asset_uid_count = 0;

int8_t asset_init();
void asset_exit();

asset_t *asset_get(const char *map_name, const char *file);
asset_t *asset_req(const char *map_name, const char *file);

asset_t *asset_get_chunk(const char *map, const char *file,
                         uint32_t chunk_start, uint32_t chunk_length);
asset_t *asset_req_chunk(const char *map, const char *file,
                         uint32_t chunk_start, uint32_t chunk_length);

asset_map_t *asset_create_map(const char *filename, const char *name,
                              uint32_t capacity, uint8_t compression_level);

void asset_update_map(asset_map_t *map);

void asset_free(asset_t *asset);
void asset_map_free(const char *map);
void asset_map_free_t(asset_map_t *map);

#ifdef __cplusplus
}
#endif

#endif
