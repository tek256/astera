// TODO file writing methods (general fs, pak, zip)

#ifndef ASTERA_ASSET_HEADER
#define ASTERA_ASSET_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>

typedef struct {
  uint32_t       uid;
  unsigned char* data;
  uint32_t       data_length;
  const char*    name;

  int32_t chunk_start, chunk_length;

  int filled : 1;
  int req : 1;
  int req_free : 1;
  int chunk : 1;
} asset_t;

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

  FILE *in, *out;

  pak_file_t* entries;
  uint32_t    count;
  int         file_mode : 1;
} pak_t;

typedef enum { MAP_FS = 0, MAP_PAK = 1, MAP_ZIP = 2 } asset_map_type;

typedef struct {
  asset_t** assets;

  uint32_t count;
  uint32_t capacity;
  uint32_t uid_counter;

  const char* name;
  const char* filename;

  pak_t* pak;

  uint8_t        compression_level;
  asset_map_type type;
} asset_map_t;

pak_t*  pak_open(const char* file, const char* mode);
int32_t pak_append(pak_t* pak, const char* filename, const unsigned char* data,
                   uint32_t data_length);
int32_t pak_append_file(pak_t* pak, const char* filename, FILE* in);
void    pak_open_mem(pak_t* pak, unsigned char* data, uint32_t data_length);
unsigned char* pak_extract(pak_t* pak, uint32_t index);
unsigned char* pak_extract_noalloc(pak_t* pak, uint32_t index,
                                   unsigned char* out, uint32_t out_cap);
pak_t*         pak_close(pak_t* pak);

int32_t  pak_find(pak_t* pak, const char* filename);
uint32_t pak_count(pak_t* pak);
uint32_t pak_offset(pak_t* pak, uint32_t index);
uint32_t pak_size(pak_t* pak, uint32_t index);
char*    pak_name(pak_t* pak, uint32_t index);

/* Create an asset map to track assets using a zip file */
asset_map_t asset_map_create_zip(const char* filename, const char* name,
                                 uint32_t capacity, uint8_t compression_level);

/* Create an asset map to track assets using a pak file */
asset_map_t asset_map_create_pak(const char* filename, const char* name,
                                 uint32_t capacity);

/* Add an asset into the tracking of a map */
void asset_map_add(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map */
void asset_map_remove(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map by id */
void asset_map_removei(asset_map_t* map, uint32_t id);
/* Get a file from the asset map's directed source */
asset_t* asset_map_get(asset_map_t* map, const char* file);

/* Update for any free requests made*/
void asset_map_update(asset_map_t* map);

/* Get a file from the local system
   file - the file path of the file
   returns: formatted asset_t struct pointer with file data */
asset_t* asset_get(const char* file);

/* Get a chunk from a local system file */
asset_t* asset_get_chunk(const char* file, uint32_t chunk_start,
                         uint32_t chunk_length);

/* Free any memory used by the asset */
void asset_free(asset_t* asset);

/* Free the map and all the assets within the map */
void asset_map_free(asset_map_t* map);

/* Write data to the file system */
int8_t asset_write(const char* file_path, void* data, uint32_t data_length);

#ifdef __cplusplus
}
#endif
#endif
