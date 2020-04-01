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
  unsigned int   uid;
  unsigned char* data;
  unsigned int   data_length;
  const char*    name;

  int32_t chunk_start, chunk_length;

  int filled : 1;
  int req : 1;
  int req_free : 1;
  int chunk : 1;
} asset_t;

typedef struct {
  asset_t*    assets;
  uint32_t    count;
  uint32_t    capacity;
  const char* name;
  const char* filename;
  uint8_t     compression_level;
  int         free : 1;
} asset_map_t;

#if !defined(ASTERA_NO_PAK)
#include <stdio.h>
typedef struct {
  char     id[4];
  uint32_t offset;
  uint32_t size;
} pak_header_t;

typedef struct {
  char     name[56];
  uint32_t offset;
  uint32_t size;
} pak_file_t;

typedef struct {
  unsigned char* data_ptr;
  uint32_t       data_size;

#if !defined(ASTERA_PAK_NO_FILE)
  FILE *in, *out;
#endif

  pak_file_t* entries;
  uint32_t    count;
#if !defined(ASTERA_PAK_NO_FILE)
  int file_mode : 1;
#endif
} pak_t;

#if !defined(ASTERA_PAK_NO_FILE)
pak_t*  pak_open(const char* file, const char* mode);
int32_t pak_append(pak_t* pak, const char* filename, const unsigned char* data,
                   uint32_t data_length);
int32_t pak_append_file(pak_t* pak, const char* filename, FILE* in);
#endif
void pak_open_mem(pak_t* pak, unsigned char* data, uint32_t data_length);
unsigned char* pak_extract(pak_t* pak, uint32_t index);
unsigned char* pak_extract_noalloc(pak_t* pak, uint32_t index,
                                   unsigned char* out, uint32_t out_cap);
pak_t*         pak_close(pak_t* pak);

int32_t  pak_find(pak_t* pak, const char* filename);
uint32_t pak_count(pak_t* pak);
uint32_t pak_offset(pak_t* pak, uint32_t index);
uint32_t pak_size(pak_t* pak, uint32_t index);
char*    pak_name(pak_t* pak, uint32_t index);

#endif

static asset_map_t  asset_maps[ASSET_MAX_MAPS];
static unsigned int asset_map_count = 0;
static unsigned int asset_uid_count = 0;

int8_t asset_init();
void   asset_exit();

asset_t* asset_get(const char* map_name, const char* file);
asset_t* asset_req(const char* map_name, const char* file);

asset_t* asset_get_chunk(const char* map, const char* file,
                         uint32_t chunk_start, uint32_t chunk_length);
asset_t* asset_req_chunk(const char* map, const char* file,
                         uint32_t chunk_start, uint32_t chunk_length);

asset_map_t* asset_create_map(const char* filename, const char* name,
                              uint32_t capacity, uint8_t compression_level);

void asset_update_map(asset_map_t* map);

void asset_free(asset_t* asset);
void asset_map_free(const char* map);
void asset_map_free_t(asset_map_t* map);

#ifdef __cplusplus
}
#endif

#endif
