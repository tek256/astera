#include <astera/asset.h>
#include <astera/debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(alloca)
#define alloca(x) __builtin_alloca(x)
#endif

static uint32_t fs_file_size(const char* fp) {
  if (!fp) {
    ASTERA_FUNC_DBG("no file path passed\n");
    return 0;
  }

  FILE* f = fopen(fp, "r");

  if (!f) {
    ASTERA_FUNC_DBG("unable to open file %s\n", fp);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  uint32_t length = (uint32_t)ftell(f);
  fclose(f);

  return length;
}

static unsigned char* fs_file_data(const char* path, uint32_t* size_ptr) {
  FILE* f = fopen(path, "rb+");

  if (!f) {
    return 0;
  }

  uint32_t data_length = 0;

  fseek(f, 0, SEEK_END);
  data_length = ftell(f);
  rewind(f);

  unsigned char* data = (unsigned char*)malloc(sizeof(char) * data_length);

  if (!data) {
    fclose(f);
    return 0;
  }

  uint32_t read_amount = (uint32_t)fread(data, sizeof(char), data_length, f);
  fclose(f);

  if (read_amount != data_length) {
    ASTERA_FUNC_DBG("read and file size different: %i vs %i\n", read_amount,
                    data_length);
  }

  if (size_ptr) {
    *size_ptr = data_length;
  }

  return data;
}

#if defined(ASTERA_PAK_WRITE)

pak_write_t* pak_write_create(const char* file) {
  pak_write_t* write = (pak_write_t*)calloc(1, sizeof(pak_write_t));

  if (!write) {
    return 0;
  }

  write->filepath = file;
  return write;
}

void pak_write_destroy(pak_write_t* write) {
  if (write->files) {
    free(write->files);
  }

  free(write);
}

static uint8_t __write_contains(pak_write_t* write, const char* name) {
  for (uint32_t i = 0; i < write->count; ++i) {
    if (!strcmp(write->files[i].name, name)) {
      return 1;
    }
  }
  return 0;
}

uint8_t pak_write_add_file(pak_write_t* write, const char* file,
                           const char* name) {
  if (!name || !file || !write) {
    return 0;
  }

  if (__write_contains(write, name)) {
    return 0;
  }

  uint32_t size = fs_file_size(file);

  if (size == 0) {
    return 0;
  }

  pak_wfile_t wfile = (pak_wfile_t){
      .data.filepath = file,
      .size          = size,
      .is_mem        = 0,
  };

  snprintf(wfile.name, 55, "%s", name);

  write->files = (pak_wfile_t*)realloc(write->files, sizeof(pak_wfile_t) *
                                                         (write->count + 1));
  write->files[write->count] = wfile;
  ++write->count;

  return 1;
}

uint8_t pak_write_add_mem(pak_write_t* write, const unsigned char* data,
                          uint32_t data_length, const char* name) {
  if (!write || !data || !data_length || !name) {
    return 0;
  }

  if (__write_contains(write, name)) {
    return 0;
  }

  uint32_t size = data_length;

  if (size == 0) {
    return 0;
  }

  pak_wfile_t wfile = (pak_wfile_t){
      .data.ptr = data,
      .size     = size,
      .is_mem   = 1,
  };

  snprintf(wfile.name, 55, "%s", name);

  write->files = (pak_wfile_t*)realloc(write->files, sizeof(pak_wfile_t) *
                                                         (write->count + 1));
  write->files[write->count] = wfile;
  ++write->count;

  return 1;
}

uint8_t pak_write_to_file(pak_write_t* write) {
  if (!write) {
    ASTERA_FUNC_DBG("no write structure passed.\n");
    return 0;
  }

  uint32_t header_offset =
      sizeof(pak_header_t) + ((sizeof(pak_file_t) * write->count));
  uint32_t offset = header_offset;

  FILE* f = fopen(write->filepath, "wb+");

  if (!f) {
    ASTERA_FUNC_DBG("unable to open file %s to write.\n", write->filepath);
    return 0;
  }

  for (uint32_t i = 0; i < write->count; ++i) {
    write->files[i].offset = offset;
    offset += write->files[i].size;
  }

  pak_header_t header = (pak_header_t){.id = "PACK", .count = write->count};

  fwrite("PACK", 1, 4, f);
  fwrite(&header.count, 1, sizeof(uint32_t), f);

  for (uint32_t i = 0; i < header.count; ++i) {
    pak_wfile_t* wfile = &write->files[i];

    fwrite(wfile->name, 1, 56, f);
    fwrite(&wfile->offset, 1, sizeof(uint32_t), f);
    fwrite(&wfile->size, 1, sizeof(uint32_t), f);
  }

  for (uint32_t i = 0; i < header.count; ++i) {
    pak_wfile_t*   wfile     = &write->files[i];
    unsigned char* file_data = 0;
    if (wfile->is_mem) {
      file_data = wfile->data.ptr;
    } else {
      file_data = fs_file_data(wfile->data.filepath, 0);
    }

    fwrite(file_data, sizeof(unsigned char), wfile->size, f);

    if (!wfile->is_mem)
      free(file_data);
  }

  fclose(f);

  return 1;
}

#endif

pak_t* pak_open_file(const char* file) {
  FILE* f = fopen(file, "rb+");

  if (!f) {
    ASTERA_FUNC_DBG("unable to open file %s\n", file);
    return 0;
  }

  pak_header_t header = {0};
  fread(&header, 1, sizeof(pak_header_t), f);

  if (strncmp(header.id, "PACK", 4)) {
    ASTERA_FUNC_DBG("invalid pak file format %s\n", file);
    fclose(f);
    return 0;
  }

  if (header.count == 0) {
    ASTERA_FUNC_DBG("empty pak file %s\n", file);
    fclose(f);
    return 0;
  }

  pak_t* pak = (pak_t*)calloc(1, sizeof(pak_t));

  if (!pak) {
    ASTERA_FUNC_DBG("unable to alloc pak_t\n");
    fclose(f);
    return 0;
  }

  pak->count         = header.count;
  pak->data.filepath = file;

  // Seek to the end of the header size (start of entries)
  fseek(f, sizeof(pak_header_t), SEEK_SET);

  pak->files = (pak_file_t*)calloc(header.count, sizeof(pak_file_t));

  if (!pak->files) {
    ASTERA_FUNC_DBG("unable to allocate space for entries\n");
    free(pak);
    return 0;
  }

  if (fread(pak->files, header.count, sizeof(pak_file_t), f) !=
      sizeof(pak_file_t)) {
    ASTERA_FUNC_DBG("invalid read of entries: %s\n", file);
    fclose(f);
    free(pak->files);
    free(pak);
    return 0;
  }

  fclose(f);

  return pak;
}

pak_t* pak_open_mem(unsigned char* data, uint32_t data_length) {
  if (!data || !data_length) {
    ASTERA_FUNC_DBG("no data passed\n");
    return 0;
  }

  // Get the header from the start of the data passed
  pak_header_t* header = (pak_header_t*)data;

  if (!header) {
    ASTERA_FUNC_DBG("unable to obtain header pointer\n");
    return 0;
  }

  if (memcmp(header->id, "PACK", 4)) {
    ASTERA_FUNC_DBG("invalid pak file\n");
    return 0;
  }

  pak_t* pak = (pak_t*)calloc(1, sizeof(pak_t));

  if (!pak) {
    ASTERA_FUNC_DBG("unable to allocate space for pak header\n");
    return 0;
  }

  pak->is_mem = 1;
  pak->files  = 0;

  pak->files = (pak_file_t*)data + sizeof(pak_header_t);

  return pak;
}

unsigned char* pak_extract(pak_t* pak, uint32_t index, uint32_t* size) {
  if (!pak) {
    ASTERA_FUNC_DBG("no pak header passed\n");
    return 0;
  }

  if (index >= pak->count) {
    ASTERA_FUNC_DBG("entry outside of pak structure entry count\n");
    return 0;
  }

  pak_file_t* entry = &pak->files[index];

  if (!pak->is_mem) {
    FILE* f = fopen(pak->data.filepath, "rb+");

    if (!f) {
      ASTERA_FUNC_DBG("unable to open pak file %s\n", pak->data.filepath);
      return 0;
    }

    unsigned char* data =
        (unsigned char*)calloc(entry->size + 1, sizeof(unsigned char));

    if (!data) {
      ASTERA_FUNC_DBG("unable to allocate %i bytes\n",
                      (sizeof(unsigned char) * entry->size));
      fclose(f);
      return 0;
    }

    fseek(f, entry->offset, SEEK_SET);
    if (!fread(data, sizeof(unsigned char), entry->size, f)) {
      ASTERA_FUNC_DBG("unable to read %i bytes\n",
                      (sizeof(unsigned char) * entry->size));
      fclose(f);
      free(data);
      return 0;
    }

    if (size) {
      *size = entry->size;
    }

    return data;
  } else {
    unsigned char* data = (unsigned char*)(pak->data.ptr + entry->offset);

    if (size) {
      *size = entry->size;
    }

    return data;
  }
}

uint32_t pak_extract_noalloc(pak_t* pak, uint32_t index, unsigned char* out,
                             uint32_t out_cap) {
  if (!out || !out_cap) {
    ASTERA_FUNC_DBG("invalid out buffer parameters passed\n");
    return 0;
  }

  if (index > pak->count) {
    ASTERA_FUNC_DBG("index outside of pak entry count\n");
    return 0;
  }

  pak_file_t* entry = &pak->files[index];

  if (!pak->is_mem) {
    FILE* f = fopen(pak->data.filepath, "rb+");

    if (!f) {
      ASTERA_FUNC_DBG("unable to open pak file %s\n", pak->data.filepath);
      return 0;
    }

    fseek(f, entry->offset, SEEK_SET);
    if (!fread(out, sizeof(unsigned char), entry->size, f)) {
      ASTERA_FUNC_DBG("unable to read %i bytes\n",
                      (sizeof(unsigned char) * entry->size));
      fclose(f);
      free(out);
      return 0;
    }

    return entry->size;
  } else {
    unsigned char* data = (unsigned char*)(pak->data.ptr + entry->offset);
    memcpy(out, data, sizeof(unsigned char) * entry->size);
    return entry->size;
  }
}

uint8_t pak_close(pak_t* pak) {
  if (!pak) {
    ASTERA_FUNC_DBG("no pak header passed.\n");
    return 0;
  }

  if (pak->is_mem) {
    free(pak->data.ptr);
  }

  if (pak->files) {
    free(pak->files);
  }

  // free out the pointer itself
  free(pak);

  return 1;
}

int32_t pak_find(pak_t* pak, const char* filename) {
  if (!pak)
    return -1;

  for (uint32_t i = 0; i < pak->count; ++i) {
    if (!strcmp(pak->files[i].name, filename)) {
      return i;
    }
  }

  return -1;
}

uint32_t pak_count(pak_t* pak) {
  if (!pak)
    return 0;

  return pak->count;
}

uint32_t pak_offset(pak_t* pak, uint32_t index) {
  if (!pak)
    return 0;
  return index < pak->count ? pak->files[index].offset : 0;
}

uint32_t pak_size(pak_t* pak, uint32_t index) {
  if (!pak)
    return 0;
  return index < pak->count ? pak->files[index].size : 0;
}

char* pak_name(pak_t* pak, uint32_t index) {
  if (!pak)
    return 0;
  return (index < pak->count) ? pak->files[index].name : 0;
}

uint32_t asset_fnv1a_init(void) {
  return (uint32_t)ASTERA_HASH_INITIAL;
}

void asset_fnv1a_hash(uint32_t* hash, const void* data, uint32_t size) {
  const unsigned char* d = data;
  while (size--) {
    *hash = (*hash ^ *d++) * 16777619;
  }
}

uint32_t asset_checksum(asset_t* asset) {
  uint32_t hash = ASTERA_HASH_INITIAL;
  asset_fnv1a_hash(&hash, asset->data, asset->data_length);
  return hash;
}

void asset_free(asset_t* asset) {
  asset->name   = 0;
  asset->filled = 0;
  asset->req    = 0;
  if (asset->data)
    free(asset->data);

  // these pointers are allocated independent of anything
  if (asset->fs)
    free(asset);
}

void asset_map_free(asset_map_t* map) {
  if (!map) {
    ASTERA_FUNC_DBG("No map passed to free.\n");
    return;
  }

  for (uint32_t i = 0; i < map->capacity; ++i) {
    asset_t* asset = map->assets[i];
    if (asset->data) {
      free(asset->data);
    }
  }

  free(map->assets);
  map->assets   = 0;
  map->count    = 0;
  map->capacity = 0;
  map->name     = 0;
  map->filename = 0;
}

asset_t* asset_map_get(asset_map_t* map, const char* file) {
  // First check if we have it already loaded
  if (map->count > 0) {
    for (uint32_t i = 0; i < map->capacity; ++i) {
      if (map->assets[i]) {
        if (map->assets[i]->name && map->assets[i]->data) {
          if (strcmp(file, map->assets[i]->name) == 0) {
            return map->assets[i];
          }
        }
      }
    }
  }

  if (map->pak) {
    asset_t* asset = (asset_t*)calloc(1, sizeof(asset_t));
    asset->name    = file;
    ++map->uid_counter;
    asset->uid = map->uid_counter;

    int32_t asset_index = pak_find(map->pak, file);

    if (asset_index == -1) {
      free(asset);
      return 0;
    }

    asset->data = pak_extract(map->pak, asset_index, &asset->data_length);

    asset->filled = 1;
    return asset;
  } else {
    asset_t* asset = asset_get(file);
    ++map->uid_counter;
    asset->uid = map->uid_counter;

    if (asset) {
      for (uint32_t i = 0; i < map->capacity; ++i) {
        if (!map->assets[i]) {
          map->assets[i] = asset;
          ++map->count;
          return asset;
        }
      }
    }
    asset->filled = 1;
    return asset;
  }
}

asset_t* asset_map_geti(asset_map_t* map, uint32_t id) {
  if (!map) {
    ASTERA_FUNC_DBG("invalid asset map\n");
    return 0;
  }

  if (id < map->capacity) {
    return map->assets[id];
  }

  return 0;
}

asset_t asset_create(void* data_ptr, uint32_t data_length) {
  return (asset_t){.data = data_ptr, .data_length = data_length, 0};
}

// NOTE: local filesystem
asset_t* asset_get(const char* file) {
  if (!file) {
    ASTERA_FUNC_DBG("no file requested.\n");
    return 0;
  }

  FILE* f = fopen(file, "r+b");

  if (!f) {
    ASTERA_FUNC_DBG("Unable to open system file: %s\n", file);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  uint32_t file_size = ftell(f);
  rewind(f);

  unsigned char* data =
      (unsigned char*)malloc(sizeof(unsigned char) * (file_size + 1));

  if (!data) {
    ASTERA_FUNC_DBG("Unable to allocate %i bytes for file %s\n", file_size,
                    file);
    fclose(f);
    return 0;
  }

  uint32_t data_read =
      (uint32_t)fread(data, sizeof(unsigned char), file_size, f);

  if (data_read != file_size) {
    ASTERA_FUNC_DBG("Incomplete read: %i expeceted, %i read.\n", file_size,
                    data_read);
    free(data);
    return 0;
  }

  // NULL-Terminate the data
  data[data_read] = 0;

  fclose(f);

  asset_t* asset     = (asset_t*)calloc(1, sizeof(asset_t));
  asset->data        = data;
  asset->data_length = data_read;

  asset->name   = file;
  asset->filled = 1;
  asset->fs     = 1;

  return asset;
}

asset_map_t asset_map_create(const char* filename, const char* name,
                             uint32_t capacity) {
  asset_map_t map = (asset_map_t){
      .capacity = capacity, .name = name, .filename = filename, 0};

  map.assets = (asset_t**)calloc(capacity, sizeof(asset_t*));

  if (filename) {
    pak_t* pak = pak_open_file(filename);

    if (!pak) {
      map.filename = 0;
    } else {
      map.pak = pak;
    }
  }

  return map;
}

asset_map_t asset_map_create_mem(unsigned char* data, uint32_t data_length,
                                 const char* name, uint32_t capacity) {
  asset_map_t map =
      (asset_map_t){.capacity = capacity, .name = name, .filename = 0, 0};

  map.assets = (asset_t**)calloc(capacity, sizeof(asset_t*));

  if (data && data_length) {
    pak_t* pak = pak_open_mem(data, data_length);

    if (!pak) {
      map.filename = 0;
    } else {
      map.pak = pak;
    }
  }

  return map;
}

void asset_map_destroy(asset_map_t* map) {
  if (!map)
    return;

  if (map->pak && map->filename) {
    pak_close(map->pak);
  }

  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (map->assets[i]) {
      asset_free(map->assets[i]);
    }
  }

  free(map->assets);
}

void asset_map_add(asset_map_t* map, asset_t* asset) {
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!map->assets[i]) {
      map->assets[i] = asset;
      asset->uid     = i;
      ++map->count;
      break;
    }
  }
}

void asset_map_remove(asset_map_t* map, asset_t* asset) {
  asset_map_removei(map, asset->uid);
}

void asset_map_removei(asset_map_t* map, uint32_t id) {
  if (!map->assets[id]) {
    ASTERA_FUNC_DBG("no asset at index %i on asset map.\n", id);
    return;
  }

  free(map->assets[id]->data);
  map->assets[id]->uid = 0;
  --map->count;
}

void asset_map_update(asset_map_t* map) {
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (map->assets[i]->filled && map->assets[i]->req_free) {
      asset_map_removei(map, i);
    }
  }
}

asset_t* asset_get_chunk(const char* file, uint32_t chunk_start,
                         uint32_t chunk_length) {
  if (!file) {
    ASTERA_FUNC_DBG("No file requested\n");
    return 0;
  }

  asset_t* asset = (asset_t*)calloc(1, sizeof(asset_t));
  FILE*    f     = fopen(file, "r+b");

  if (!f) {
    ASTERA_FUNC_DBG("Unable to open system file: %s\n", file);
    free(asset);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  uint32_t file_size = ftell(f);
  rewind(f);

  if (chunk_start > file_size) {
    ASTERA_FUNC_DBG("Chunk requested starts out of bounds [%i] of the "
                    "file [%i].\n",
                    chunk_start, file_size);
    asset->req   = 0;
    asset->chunk = 0;
    return 0;
  }

  // Allow for size clamping, in case we just want the data and not to have
  // to analyze it prior
  uint32_t max_length = (chunk_start + chunk_length > file_size)
                            ? file_size - chunk_start
                            : chunk_length;

  unsigned char* data =
      (unsigned char*)malloc(sizeof(unsigned char) * (max_length + 1));

  if (!data) {
    ASTERA_FUNC_DBG("Unable to allocate [%i] bytes for file chunk %s\n",
                    max_length, file);
    free(asset);
    fclose(f);
    return 0;
  }

  uint32_t data_read =
      (uint32_t)fread(data, sizeof(unsigned char), max_length, f);

  if (data_read != max_length) {
    ASTERA_FUNC_DBG("Incomplete read: %i expeceted, %i read.\n", max_length,
                    data_read);
    free(asset);
    free(data);
    return 0;
  }

  data[data_read] = 0;

  fclose(f);

  asset->data_length = data_read;
  asset->data        = data;

  asset->name        = file;
  asset->filled      = 1;
  asset->chunk       = 1;
  asset->chunk_start = chunk_start;

  return asset;
}

/* Write data to the file system */
uint8_t asset_write_data(const char* file_path, void* data,
                         uint32_t data_length) {
  FILE* f = fopen(file_path, "wb");

  if (!f) {
    return 0;
  }

  uint32_t write_length = (uint32_t)fwrite(data, data_length, 1, f);

  fclose(f);
  if (write_length != data_length) {
    ASTERA_FUNC_DBG("mismatched write & data sizes [data_length: %i, "
                    "write_length: %i].\n",
                    data_length, write_length);
    return 0;
  }

  return 1;
}

uint8_t asset_write(const char* filepath, asset_t* asset) {
  return asset_write_data(filepath, asset->data, asset->data_length);
}

