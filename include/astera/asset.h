#ifndef ASTERA_ASSET_HEADER
#define ASTERA_ASSET_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || \
    defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64) ||     \
    defined(__aarch64__)
#define ASTERA_ARCH_64BIT
#elif defined(i386) || defined(__i386) || defined(__i386__) ||     \
    defined(__i486__) || defined(__i586__) || defined(__i686__) || \
    defined(__IA32__) || defined(_M_XI86) || defined(__X86__) ||   \
    defined(_X86_)
#define ASTERA_ARCH_32BIT
#endif

#include <stdint.h>

#define ASTERA_HASH_INITIAL 2166136261

typedef struct {
  uint32_t       uid;
  unsigned char* data;
  uint32_t       data_length;
  const char*    name;

  int32_t chunk_start, chunk_length;

  uint8_t fs;
  uint8_t filled;
  uint8_t req;
  uint8_t req_free;
  uint8_t chunk;
} asset_t;

#if defined(ASTERA_PAK_WRITE)
typedef struct {
  char     name[56];
  uint32_t offset;
  uint32_t size;
  uint32_t index;

  // If  the file is loaded in memory or not
  uint8_t is_mem;

  union {
    const char* filepath;
    void*       ptr;
  } data;
} pak_wfile_t;

typedef struct {
  const char*  filepath;
  pak_wfile_t* files;
  uint32_t     count;
} pak_write_t;
#endif

/* PAK FILE LAYOUT:
 * 0  - pack_header_t
 * 12 - entries
 * n  - start of files */

typedef struct {
  char     id[4];
  uint32_t count;
} pak_header_t;

// exactly 64 bytes
typedef struct {
  char     name[56];
  uint32_t offset;
  uint32_t size;
} pak_file_t;

typedef struct {
  pak_file_t* files;
  uint32_t    count;

  union {
    const void* ptr;
    const char* filepath;
  } data;

  /* data_size - the size of the pak file (bytes)
   * is_mem - if to use data.ptr for accessing data */
  uint32_t data_size;
  uint8_t  is_mem;
} pak_t;

typedef struct {
  asset_t** assets;

  uint32_t count;
  uint32_t capacity;

  // uid_counter - the ID of assets in this map
  uint32_t uid_counter;

  const char* name;
  const char* filename;

  pak_t* pak;
} asset_map_t;

#if defined(ASTERA_PAK_WRITE)
/* Create a write structure to create a pak file
 * file - the output path for the pak file
 * returns: formatted structure for creating pak files
 * NOTE: THIS FUNCTION IS ONLY INCLUDED IF ASTERA_PAK_WRITE IS DEFINED */
pak_write_t* pak_write_create(const char* file);

/* Destroy a write structure
 * write - the structure to free
 * NOTE: THIS FUNCTION IS ONLY INCLUDED IF ASTERA_PAK_WRITE IS DEFINED */
void pak_write_destroy(pak_write_t* write);

/* Add a file to the pack file with name
 * write - the write structure to append to
 * file - the source file
 * name - the name to use in pack file
 * returns: success = 1, fail = 0
 * NOTE: THIS FUNCTION IS ONLY INCLUDED IF ASTERA_PAK_WRITE IS DEFINED */
uint8_t pak_write_add_file(pak_write_t* write, const char* file,
                           const char* name);

/* Add memory to the pack file with name
 * write - the write structure to append to
 * data - the data to add
 * data_length - the length of the data (in bytes)
 * name - the name of the data to use in the pack file
 * returns: success = 1, fail = 0
 * NOTE: THIS FUNCTION IS ONLY INCLUDED IF ASTERA_PAK_WRITE IS DEFINED */
uint8_t pak_write_add_mem(pak_write_t* write, const unsigned char* data,
                          uint32_t data_length, const char* name);

/* Write the pack write structure to file
 * write - the structure to write
 * returns: success = 1, fail = 0
 * NOTE: THIS FUNCTION IS ONLY INCLUDED IF ASTERA_PAK_WRITE IS DEFINED */
uint8_t pak_write_to_file(pak_write_t* write);
#endif

/* Open a file as a pak in file mode
 * file - path to the pak file
 * return: pointer to pak_t struct for usage, fail = 0 */
pak_t* pak_open_file(const char* file);

/* Open a pak structure from memory
 * pak - the destination of the pak structure
 * data - the pointer to the data of the structure
 * data_length - the length of the data
 * returns: pointer to pak_t struct for usage, fail = 0
 * NOTE: the lifetime of this data pointer is the same as the whole
 * pak structure, don't free it if you still want to use the pak struct */
pak_t* pak_open_mem(unsigned char* data, uint32_t data_length);

/* Close out a pak file & free all its resources
 *  returns: success = 1, fail = 0 */
uint8_t pak_close(pak_t* pak);

/* Get the data of an indexed file (allocates own space if filemode)
 * pak - the pak structure pointer
 * index - the index of the file
 * size - a pointer to int to set the size */
unsigned char* pak_extract(pak_t* pak, uint32_t index, uint32_t* size);

/* Get the data of an indexed file (uses your data pointer)
 * pak - the pak structure pointer
 * index - the index of the file
 * out - the allocated space to put the data in
 * out_cap - the capacity of the out space
 * used - the amount of used
 * returns: number of bytes used / read, fail = 0 */
uint32_t pak_extract_noalloc(pak_t* pak, uint32_t index, unsigned char* out,
                             uint32_t out_cap);

/* Find an entry in the pak file by name
 * pak - the pak structure
 * filename - the name of the entry
 * returns: index, fail = -1 */
int32_t pak_find(pak_t* pak, const char* filename);

/* Return the total number of entries in the pak structure
 * pak - pak file structure
 * returns: entry count */
uint32_t pak_count(pak_t* pak);

/* Get the true offset of a file within the pak file
 * pak - pak file structure
 * index - the index of the entry
 * returns: offset */
uint32_t pak_offset(pak_t* pak, uint32_t index);

/* Get the size of a file within the pak file
 * pak - pak file structure
 * index - the index of the entry
 * returns: size of the entry file */
uint32_t pak_size(pak_t* pak, uint32_t index);

/* Get the name of an entry by index
 * pak - the pak file structure
 * index - the index of the entry
 * returns: name string */
char* pak_name(pak_t* pak, uint32_t index);

/* Initialize an fnv-1a hash
 * Returns: initial fnv1a value */
uint32_t asset_fnv1a_init(void);

/* Hash on a checksum for data
 * hash - hash value to update
 * data - data to account for
 * size - the amount of data to account for */
void asset_fnv1a_hash(uint32_t* hash, const void* data, uint32_t size);

/* Create a checksum for an asset
 * asset - the asset to create a checksum for
 * returns: 32-bit checksum */
uint32_t asset_checksum(asset_t* asset);

/* Create an asset map
 * filename - point to a file to read from [OPTIONAL]
 * name - the name of the asset map capacity - the max number of
 * assets to store in the map
 * returns: formatted asset_map_t type with assets loaded in */
asset_map_t asset_map_create(const char* filename, const char* name,
                             uint32_t capacity);

/* Create an asset map
 * data - the data of a pak file [OPTIONAL]
 * data_length - the length of the pak file's data [OPTIONAL]
 * name - the name of the asset map capacity - the max number of
 * assets to store in the map
 * returns: formatted asset_map_t type with assets loaded in */
asset_map_t asset_map_create_mem(unsigned char* data, uint32_t data_length,
                                 const char* name, uint32_t capacity);

/* Destroy an asset map and all its resources
 * map - the map to destroy */
void asset_map_destroy(asset_map_t* map);

/* Add an asset into the tracking of a map */
void asset_map_add(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map */
void asset_map_remove(asset_map_t* map, asset_t* asset);
/* Remove an asset from the tracking of a map by id */
void asset_map_removei(asset_map_t* map, uint32_t id);
/* Get a file from the asset map's directed source */
asset_t* asset_map_get(asset_map_t* map, const char* file);
/* Get an asset from the map by index */
asset_t* asset_map_geti(asset_map_t* map, uint32_t id);

/* Update for any free requests made*/
void asset_map_update(asset_map_t* map);

/* Get a file from the local system
   file - the file path of the file
   returns: formatted asset_t struct pointer with file data */
asset_t* asset_get(const char* file);

/* Create an asset struct from a memory pointer
 * data_ptr - pointer to the data
 * data_length - the length of the data
 * returns: formatted asset_t struct for usage */
asset_t asset_create(void* data_ptr, uint32_t data_length);

/* Get a chunk from a local system file */
asset_t* asset_get_chunk(const char* file, uint32_t chunk_start,
                         uint32_t chunk_length);

/* Free any memory used by the asset */
void asset_free(asset_t* asset);

/* Free the map and all the assets within the map */
void asset_map_free(asset_map_t* map);

/* Write data to the file system */
uint8_t asset_write_data(const char* file_path, void* data,
                         uint32_t data_length);

/* Write an asset & its data to the file system */
uint8_t asset_write(const char* filepath, asset_t* asset);

#ifdef __cplusplus
}
#endif
#endif
