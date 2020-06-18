// TODO: Combine pak_t & asset_map_t
/*
 * NOTE: The PAK System is inspired by the r-lyeh/sdarc.c implementation
 */
#include <astera/asset.h>
#include <astera/debug.h>

#include <zip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(alloca)
#define alloca(x) __builtin_alloca(x)
#endif

#include <sys/stat.h>

static inline uint32_t pak_swap32(uint32_t t) {
  return (t >> 24) | (t << 24) | ((t >> 8) & 0xff00) | ((t & 0xff00) << 8);
}

#if defined(_M_IX86) || defined(_M_X64) // #ifdef LITTLE
#define htob32(x) pak_swap32(x)
#define btoh32(x) pak_swap32(x)
#define htol32(x) (x)
#define ltoh32(x) (x)
#else
#define htob32(x) (x)
#define btoh32(x) (x)
#define htol32(x) pak_swap32(x)
#define ltoh32(x) pak_swap32(x)
#endif

pak_t* pak_open(const char* file, const char* mode) {
  struct stat _filestats;
  int         exists = (stat(file, &_filestats) == 0);
  if (mode[0] == 'a' && !exists)
    mode = "wb";
  if (mode[0] != 'w' && mode[0] != 'r' && mode[0] != 'a')
    return 0;

  FILE* fp = fopen(file, mode[0] == 'w' ? "wb" : mode[0] == 'r' ? "rb" : "r+b");
  if (!fp) {
    ASTERA_DBG("pak_open: unable to open pak file %s\n", file);
    return 0;
  }

  pak_t *pak = (pak_t*)malloc(sizeof(pak_t)), zero = {0};

  if (!pak) {
    ASTERA_DBG("pak_open: unable to alloc pak_t.\n");
    fclose(fp);
    return 0;
  }

  memset(pak, 0, sizeof(pak_t));
  pak->file_mode = 1;

  if (mode[0] == 'r' || mode[0] == 'a') {
    pak_header_t header = (pak_header_t){0};

    if (fread(&header, 1, sizeof(pak_header_t), fp) != sizeof(pak_header_t)) {
      ASTERA_DBG("Pack File Read Error: %s\n", file);
      return 0;
    }

    if (memcmp(header.id, "PACK", 4)) {
      ASTERA_DBG("Not a valid pack file: %s\n", file);
      return 0;
    }

    header.offset = ltoh32(header.offset);
    header.size   = ltoh32(header.size);

    uint32_t num_files = header.size / sizeof(pak_file_t);

    if (fseek(fp, header.offset, SEEK_SET) != 0) {
      ASTERA_DBG("Read Error: %s\n", file);
      free(pak);
      fclose(fp);
      return 0;
    }

    pak->count   = num_files;
    pak->entries = (pak_file_t*)malloc(sizeof(pak_file_t) * num_files);
    memset(pak->entries, 0, sizeof(pak_file_t) * num_files);

    if (fread(pak->entries, num_files, sizeof(pak_file_t), fp) !=
        sizeof(pak_file_t)) {
      ASTERA_DBG("Invalid Read of entries: %s\n", file);
      fclose(fp);
      free(pak->entries);
      free(pak);
      return 0;
    }

    for (uint32_t i = 0; i < num_files; ++i) {
      pak_file_t* f = &pak->entries[i];
      f->offset     = ltoh32(f->offset);
      f->size       = ltoh32(f->size);
    }

    if (mode[0] == 'a') {
      size_t resize = header.offset;
      int    fd     = fileno(fp);
      if (fd != -1) {
#ifdef _WIN32
        int ok = 0 == _chsize_s(fd, resize);
#else
        int ok = 0 == ftruncate(fd, (off_t)resize);
#endif
        fflush(fp);
        fseek(fp, 0L, SEEK_END);
      }

      pak->out = fp;
      pak->in  = 0;
    } else {
      pak->in = fp;
    }

    return pak;
  }

  if (mode[0] == 'w') {
    pak->out        = fp;
    char header[12] = {0};
    if (fwrite(header, 1, 12, pak->out) != 12) {
      ASTERA_DBG("Unable to write temporary header for file %s.\n", file);
      fclose(fp);
      return 0;
    }

    return pak;
  }

  return 0;
}

int32_t pak_append(pak_t* pak, const char* filename, const unsigned char* data,
                   uint32_t data_length) {
  if (!data || !data_length) {
    ASTERA_DBG("pak_append: invalid data pointer passed.\n");
    return -1;
  }

  if (!pak->out || !pak->file_mode) {
    ASTERA_DBG("pak_append: unable to append to read-only pak file.\n");
    return -1;
  }

  if (pak->file_mode && pak->out) {
    uint32_t index = pak->count++;
    pak->entries   = realloc(pak->entries, pak->count * sizeof(pak_file_t));

    if (!pak->entries) {
      ASTERA_DBG("pak_append: unable to reallocate entries.\n");
      return -1;
    }

    pak_file_t *entry = &pak->entries[index], zero = {0};
    *entry = zero;
    snprintf(entry->name, 55, "%s", filename);
    entry->size   = data_length;
    entry->offset = ftell(pak->out);

    fwrite(data, 1, data_length, pak->out);

    return !ferror(pak->out);
  }

  return -1;
}

int32_t pak_append_file(pak_t* pak, const char* filename, FILE* in) {
  if (!pak) {
    ASTERA_DBG("pak_append_file: no pak struct passed.\n");
    return -1;
  }

  if (!filename) {
    ASTERA_DBG("pak_append_file: no filename passed.\n");
    return -1;
  }

  if (!in) {
    ASTERA_DBG("pak_append_file: no in file passed.\n");
    return -1;
  }

  uint32_t index = pak->count++;

  pak->entries =
      (pak_file_t*)realloc(pak->entries, pak->count * sizeof(pak_file_t));
  pak_file_t *entry = &pak->entries[index], zero = {0};
  *entry = zero;

  sprintf(entry->name, 55, "%s", filename);
  entry->offset = ftell(pak->out);

  char buf[1 << 15];
  while (!feof(in) && !ferror(in)) {
    size_t byte_count = fread(buf, 1, sizeof(buf), in);
    fwrite(buf, 1, byte_count, pak->out);
  }

  entry->size = ftell(pak->out) - entry->offset;

  return !ferror(pak->out);
}

void pak_open_mem(pak_t* pak, unsigned char* data, uint32_t data_length) {
  if (!data || !data_length) {
    ASTERA_DBG("pak_open_mem: invalid data arguments passed.\n");
    return;
  }

  memset(pak, 0, sizeof(pak_t));

  pak->file_mode = 0;

  pak_header_t* header = (pak_header_t*)data;
  if (!header) {
    ASTERA_DBG("pak_open_mem: unable to obtain header pointer.\n");
    return;
  }

  if (memcmp(header->id, "PACK", 4)) {
    ASTERA_DBG("pak_open_mem: invalid pak file.\n");
    return;
  }

  header->offset = ltoh32(header->offset);
  header->size   = ltoh32(header->size);

  uint32_t num_files = header->size / sizeof(pak_file_t);

  pak_file_t* start = (pak_file_t*)&data[header->offset];
  if (!start) {
    ASTERA_DBG("pak_open_mem: unable to seek to start of data.\n");
    return;
  }

  pak->count   = num_files;
  pak->entries = (pak_file_t*)malloc(sizeof(pak_file_t) * num_files);

  if (!pak->entries) {
    ASTERA_DBG("pak_open_mem: unable to malloc %i bytes.\n",
               (sizeof(pak_file_t) * num_files));
    return;
  }

  memset(pak->entries, 0, sizeof(pak_file_t) * num_files);

  for (uint32_t i = 0; i < num_files; ++i) {
    pak_file_t* start_ptr = (pak_file_t*)&start[i];
    pak_file_t* pak_ptr   = (pak_file_t*)&pak->entries[i];

    if (!start_ptr || !pak_ptr) {
      ASTERA_DBG("pak_open_mem: invalid read of entries.\n");
      free(pak->entries);
      return;
    }

    pak->entries[i] = start[i];
    pak_ptr->offset = ltoh32(start_ptr->offset);
    pak_ptr->size   = ltoh32(start_ptr->size);
  }
}

unsigned char* pak_extract(pak_t* pak, uint32_t index) {
  if (index < pak->count) {
    pak_file_t* file = &pak->entries[index];
    if (pak->file_mode && pak->in) {
      if (fseek(pak->in, file->offset, SEEK_SET) != 0) {
        ASTERA_DBG("File read error: cannot seek to offset %i\n", file->offset);
        return 0;
      }

      unsigned char* buffer = (unsigned char*)malloc(sizeof(char) * file->size);
      if (!buffer) {
        ASTERA_DBG("File read error: cannot malloc %i bytes.\n", file->size);
        return 0;
      }

      if (fread(buffer, 1, file->size, pak->in) != file->size) {
        ASTERA_DBG("File read error: invalid read size.\n");
        return 0;
      }

      return buffer;
    } else if (!pak->file_mode && pak->data_ptr) {
      unsigned char* offset = &pak->data_ptr[file->offset];

      if (!offset) {
        ASTERA_DBG("Memory read error: offset out of bounds: %i\n",
                   file->offset);
        return 0;
      }

      unsigned char* buffer = (unsigned char*)malloc(sizeof(char) * file->size);

      if (!buffer) {
        ASTERA_DBG("Memory read error: unable to malloc %i bytes.\n",
                   file->size);
        return 0;
      }

      memcpy(buffer, offset, sizeof(char) * file->size);

      return buffer;
    }
  }
  return 0;
}

unsigned char* pak_extract_noalloc(pak_t* pak, uint32_t index,
                                   unsigned char* out, uint32_t out_cap) {
  if (!out || !out_cap) {
    ASTERA_DBG("pak_extract_noalloc: invalid out buffer parameters passed.\n");
    return 0;
  }

  if (index < pak->count) {
    pak_file_t* file = &pak->entries[index];

    if (file->size > out_cap) {
      ASTERA_DBG("pak_extract_no_alloc: out_cap too small to hold %i bytes.\n",
                 file->size);
      return 0;
    }

    if (pak->file_mode && pak->in) {
      if (fseek(pak->in, file->offset, SEEK_SET) != 0) {
        ASTERA_DBG("File read error: cannot seek to offset %i\n", file->offset);
        return 0;
      }

      if (fread(out, 1, file->size, pak->in) != file->size) {
        ASTERA_DBG("File read error: invalid read size.\n");
        return 0;
      }

      return out;
    } else if (!pak->file_mode && pak->data_ptr) {
      unsigned char* offset = &pak->data_ptr[file->offset];

      if (!offset) {
        ASTERA_DBG("Memory read error: offset out of bounds: %i\n",
                   file->offset);
        return 0;
      }

      memcpy(out, offset, sizeof(char) * file->size);

      return out;
    }
  }
  return 0;
}

pak_t* pak_close(pak_t* pak) {
  if (pak->out) {
    uint32_t seek   = 12;
    uint32_t dirpos = (uint32_t)ftell(pak->out);
    uint32_t dirlen = pak->count * 64;
    for (uint32_t i = 0; i < pak->count; ++i) {
      pak_file_t* file = &pak->entries[i];

      // write the filename + trailing zeroes
      char zero[56] = {0};
      int  namelen  = strlen(file->name);

      fwrite(file->name, 1, namelen >= 56 ? 55 : namelen, pak->out);
      fwrite(zero, 1, namelen >= 56 ? 1 : 56 - namelen, pak->out);

      // write the file size & offset
      uint32_t pseek = htol32(seek);
      uint32_t psize = htol32(file->size);
      fwrite(&pseek, 1, 4, pak->out);
      fwrite(&psize, 1, 4, pak->out);
      seek += file->size;
    }

    // rewind & update header
    fseek(pak->out, 0, SEEK_SET);
    fwrite("PACK", 1, 4, pak->out);

    dirpos = htol32(dirpos);
    dirlen = htol32(dirlen);

    fwrite(&dirpos, 1, 4, pak->out);
    fwrite(&dirlen, 1, 4, pak->out);
  }

  if (pak->in)
    fclose(pak->in);
  if (pak->out)
    fclose(pak->out);

  if (pak->data_ptr) {
    free(pak->data_ptr);
  }

  if (pak->entries) {
    free(pak->entries);
  }

  return pak;
}

int32_t pak_find(pak_t* pak, const char* filename) {
  if ((pak->file_mode && pak->in) || (!pak->file_mode && pak->data_ptr)) {
    for (uint32_t i = 0; i < pak->count; ++i) {
      if (!strcmp(pak->entries[i].name, filename)) {
        return i;
      }
    }
  }
  return -1;
}

uint32_t pak_count(pak_t* pak) {
  if (!pak)
    return 0;
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode))
             ? pak->count
             : 0;
}

uint32_t pak_offset(pak_t* pak, uint32_t index) {
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode)) &&
                 index < pak->count
             ? pak->entries[index].offset
             : 0;
}

uint32_t pak_size(pak_t* pak, uint32_t index) {
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode)) &&
                 index < pak->count
             ? pak->entries[index].size
             : 0;
}

char* pak_name(pak_t* pak, uint32_t index) {
  return (index < pak->count) && ((pak->file_mode && pak->in) ||
                                  (!pak->file_mode && pak->data_ptr))
             ? pak->entries[index].name
             : 0;
  return 0;
}

void asset_free(asset_t* asset) {
  asset->name   = 0;
  asset->filled = 0;
  asset->req    = 0;
  free(asset->data);
}

void asset_map_free(asset_map_t* map) {
  if (!map) {
    ASTERA_DBG("No map passed to free.\n");
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

  if (map->type != MAP_FS) {
    asset_t* asset = (asset_t*)malloc(sizeof(asset_t));
    asset->name    = file;
    ++map->uid_counter;
    asset->uid      = map->uid_counter;
    asset->req      = 0;
    asset->req_free = 0;

    if (map->type == MAP_PAK) {
      int32_t asset_index = pak_find(map->pak, file);

      if (asset_index == -1) {
        free(asset);
        return 0;
      }

      asset->data_length = pak_size(map->pak, asset_index);
      asset->data        = pak_extract(map->pak, asset_index);
    } else if (map->type == MAP_ZIP) {
    }

    asset->filled = 1;
    return asset;
  } else {
    asset_t* asset = asset_get(file);
    ++map->uid_counter;
    asset->uid = map->uid_counter;

    if (asset) {
      int8_t isset = 0;
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

// NOTE: Local System
asset_t* asset_get(const char* file) {
  if (!file) {
    ASTERA_DBG("asset_get: no file requested.\n");
    return 0;
  }

  asset_t* asset = (asset_t*)malloc(sizeof(asset_t));

  FILE* f = fopen(file, "r+b");

  if (!f) {
    ASTERA_DBG("Unable to open system file: %s\n", file);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  uint32_t file_size = ftell(f);
  rewind(f);

  unsigned char* data =
      (unsigned char*)malloc(sizeof(unsigned char) * (file_size + 1));

  if (!data) {
    ASTERA_DBG("Unable to allocate %i bytes for file %s\n", file_size, file);
    free(asset);
    fclose(f);
    return 0;
  }

  uint32_t data_read = fread(data, sizeof(unsigned char), file_size, f);

  if (data_read != file_size) {
    ASTERA_DBG("Incomplete read: %i expeceted, %i read.\n", file_size,
               data_read);
    free(data);
    free(asset);
    return 0;
  }

  // NULL-Terminate the data
  data[data_read] = 0;

  fclose(f);

  asset->data        = data;
  asset->data_length = data_read;

  asset->name     = file;
  asset->filled   = 1;
  asset->req      = 0;
  asset->req_free = 0;
  asset->chunk    = 0;

  return asset;
}

void asset_map_add(asset_map_t* map, asset_t* asset) {
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!map->assets[i]) {
      map->assets[i] = asset;
      asset->uid     = i;
      break;
    }
  }
}

void asset_map_remove(asset_map_t* map, asset_t* asset) {
  asset_map_removei(map, asset->uid);
}

void asset_map_removei(asset_map_t* map, uint32_t id) {
  if (!map->assets[id]) {
    ASTERA_DBG("asset_map_removei: no asset at index %i on asset map.\n", id);
    return;
  }

  free(map->assets[id]->data);
  map->assets[id]->uid = 0;
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
    ASTERA_DBG("No file requested\n");
    return 0;
  }

  asset_t* asset = (asset_t*)malloc(sizeof(asset_t));
  FILE*    f     = fopen(file, "r+b");

  if (!f) {
    ASTERA_DBG("asset_get_chunk: Unable to open system file: %s\n", file);
    free(asset);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  uint32_t file_size = ftell(f);
  rewind(f);

  if (chunk_start > file_size) {
    ASTERA_DBG(
        "asset_get_chunk: Chunk requested starts out of bounds [%i] of the "
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
    ASTERA_DBG("Unable to allocate [%i] bytes for file chunk %s\n", max_length,
               file);
    free(asset);
    fclose(f);
    return 0;
  }

  uint32_t data_read = fread(data, sizeof(unsigned char), max_length, f);

  if (data_read != max_length) {
    ASTERA_DBG("Incomplete read: %i expeceted, %i read.\n", max_length,
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
  asset->req         = 0;
  asset->req_free    = 0;

  return asset;
}

asset_map_t asset_map_create(const char* filename, const char* name,
                             uint32_t capacity, uint8_t compression_level,
                             asset_map_type type) {
  asset_map_t map =
      (asset_map_t){.assets   = (asset_t**)malloc(sizeof(asset_t*) * capacity),
                    .count    = 0,
                    .capacity = capacity,
                    .name     = name,
                    .filename = filename,
                    .compression_level = compression_level,
                    .type              = type};

  return map;
}

/* Write data to the file system */
int8_t asset_write(const char* file_path, void* data, uint32_t data_length) {
  FILE* f = fopen(file_path, "wb");

  if (!f) {
    return -1;
  }

  uint32_t write_length = fwrite(data, data_length, 1, f);

  if (write_length != data_length) {
    ASTERA_DBG("asset_write: mismatched write & data sizes [data_length: %i, "
               "write_length: %i].\n",
               data_length, write_length);
  }

  fclose(f);
}
