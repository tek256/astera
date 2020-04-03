#ifndef ASTERA_ASSET_HEADER
#define ASTERA_ASSET_HEADER

#ifdef __cplusplus
extern "C" {
#endif

//#define ASSET_NO_SYS_MAP -- this one is a tricky one, it'll break _everything_
// but I don't know that I want to have to require system map

// Alright, I'm gonna do this a specific way now,
// I have the git history in case I Want to revert it,
// but I'm essentially going to remove any hints of statically defined asset
// maps Everything will be call response, no request / queue involved It'll make
// things easier to support in the long run, and I can always provide examples
// later on on how to implement this in games for others tho, it's not terribly
// important that it exist in other's games for them. Often times dynamic
// resource loading is easier done with straight call response

// So, goodbye to the cache!

#include <stdint.h>

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

typedef enum { MAP_FS = 0, MAP_PAK = 1, MAP_ZIP = 2 } asset_map_type;

typedef struct {
  asset_t** assets;

  uint32_t count;
  uint32_t capacity;

  const char* name;
  const char* filename;

#if !defined(ASTERA_NO_PAK)
  pak_t* pak;
#endif

  uint8_t        compression_level;
  asset_map_type type;
} asset_map_t;

/* Create an asset map to track assets */
asset_map_t asset_map_create(const char* filename, const char* name,
                             uint32_t capacity, uint8_t compression_level,
                             asset_map_type type);

/* Check asset map for a file
 * if allow_fetch: go and load the data immediately */
asset_t* asset_map_find(asset_map_t* map, const char* name, int allow_fetch);

/* Add an asset into the tracking of a map */
void asset_map_add(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map */
void asset_map_remove(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map by id */
void asset_map_removei(asset_map_t* map, uint32_t id);

/* Update for any free requests made*/
void asset_map_update(asset_map_t* map);

/* Get a file from the local system */
asset_t* asset_get(const char* file);

/* Get a chunk from a local system file */
asset_t* asset_get_chunk(const char* file, uint32_t chunk_start,
                         uint32_t chunk_length);

/* Free any memory used by the asset */
void asset_free(asset_t* asset);

/* Free the map and all the assets within the map */
void asset_map_free(asset_map_t* map);

#ifdef __cplusplus
}
#endif
#endif
