#ifndef ASTERA_ASSET_HEADER
#define ASTERA_ASSET_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/* Debug Output Macro*/
#if defined ASTERA_DEBUG_OUTPUT
#if defined ASTERA_DEBUG_INCLUDED
#define DBG_E(fmt, ...) _l(fmt, ##__VA_ARGS_)
#else
#include <stdio.h>
#define DBG_E(fmt, ...) printf(fmt, ##__VA_ARGS_)
#endif
#else
#if !defined DBG_E
#define DBG_E(fmt, ...)
#endif
#endif

#include <astera/export.h>
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
ASTERA_API pak_t*  pak_open(const char* file, const char* mode);
ASTERA_API int32_t pak_append(pak_t* pak, const char* filename,
                              const unsigned char* data, uint32_t data_length);
ASTERA_API int32_t pak_append_file(pak_t* pak, const char* filename, FILE* in);
#endif
ASTERA_API void           pak_open_mem(pak_t* pak, unsigned char* data,
                                       uint32_t data_length);
ASTERA_API unsigned char* pak_extract(pak_t* pak, uint32_t index);
ASTERA_API unsigned char* pak_extract_noalloc(pak_t* pak, uint32_t index,
                                              unsigned char* out,
                                              uint32_t       out_cap);
ASTERA_API pak_t* pak_close(pak_t* pak);

ASTERA_API int32_t  pak_find(pak_t* pak, const char* filename);
ASTERA_API uint32_t pak_count(pak_t* pak);
ASTERA_API uint32_t pak_offset(pak_t* pak, uint32_t index);
ASTERA_API uint32_t pak_size(pak_t* pak, uint32_t index);
ASTERA_API char*    pak_name(pak_t* pak, uint32_t index);

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
ASTERA_API asset_map_t asset_map_create(const char* filename, const char* name,
                                        uint32_t       capacity,
                                        uint8_t        compression_level,
                                        asset_map_type type);

/* Check asset map for a file
 * if allow_fetch: go and load the data immediately */
ASTERA_API asset_t* asset_map_find(asset_map_t* map, const char* name,
                                   int allow_fetch);

/* Add an asset into the tracking of a map */
ASTERA_API void asset_map_add(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map */
ASTERA_API void asset_map_remove(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map by id */
ASTERA_API void asset_map_removei(asset_map_t* map, uint32_t id);

/* Update for any free requests made*/
ASTERA_API void asset_map_update(asset_map_t* map);

/* Get a file from the local system */
ASTERA_API asset_t* asset_get(const char* file);

/* Get a chunk from a local system file */
ASTERA_API asset_t* asset_get_chunk(const char* file, uint32_t chunk_start,
                                    uint32_t chunk_length);

/* Free any memory used by the asset */
ASTERA_API void asset_free(asset_t* asset);

/* Free the map and all the assets within the map */
ASTERA_API void asset_map_free(asset_map_t* map);

#ifdef __cplusplus
}
#endif
#endif
