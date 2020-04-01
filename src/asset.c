#include "asset.h"
#include "debug.h"

#if !defined(ASTERA_NO_ZIP)
#include <zip.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if !defined(alloca)
#define alloca(x) __builtin_alloca(x)
#endif

/*
 * NOTE: The PAK System is inspired by the r-lyeh/sdarc.c implementation
 */

#if !defined(ASTERA_NO_PAK)
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

#define ASTERA_DEBUG_OUTPUT

#if !defined(ASTERA_NO_PAK_FILE)
pak_t* pak_open(const char* file, const char* mode) {
  struct stat _filestats;
  int         exists = (stat(file, &_filestats) == 0);
  if (mode[0] == 'a' && !exists)
    mode = "wb";
  if (mode[0] != 'w' && mode[0] != 'r' && mode[0] != 'a')
    return NULL;

  FILE* fp = fopen(file, mode[0] == 'w' ? "wb" : mode[0] == 'r' ? "rb" : "r+b");
  if (!fp) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open: unable to open pak file %s\n", file);
#endif
    return 0;
  }

  pak_t *pak = (pak_t*)malloc(sizeof(pak_t)), zero = {0};

  if (!pak) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open: unable to alloc pak_t.\n");
#endif
    fclose(fp);
    return 0;
  }

  memset(pak, 0, sizeof(pak_t));
  pak->file_mode = 1;

  if (mode[0] == 'r' || mode[0] == 'a') {
    pak_header_t header = (pak_header_t){0};

    if (fread(&header, 1, sizeof(pak_header_t), fp) != sizeof(pak_header_t)) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("Pack File Read Error: %s\n", file);
#endif
      return 0;
    }

    if (memcmp(header.id, "PACK", 4)) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("Not a valid pack file: %s\n", file);
#endif
      return 0;
    }

    header.offset = ltoh32(header.offset);
    header.size   = ltoh32(header.size);

    uint32_t num_files = header.size / sizeof(pak_file_t);

    if (fseek(fp, header.offset, SEEK_SET) != 0) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("Read Error: %s\n", file);
#endif
      free(pak);
      fclose(fp);
      return 0;
    }

    pak->count   = num_files;
    pak->entries = (pak_file_t*)malloc(sizeof(pak_file_t) * num_files);
    memset(pak->entries, 0, sizeof(pak_file_t) * num_files);

    if (fread(pak->entries, num_files, sizeof(pak_file_t), fp) !=
        sizeof(pak_file_t)) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("Invalid Read of entries: %s\n", file);
#endif
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
      pak->in  = NULL;
    } else {
      pak->in = fp;
    }

    return pak;
  }

  if (mode[0] == 'w') {
    pak->out        = fp;
    char header[12] = {0};
    if (fwrite(header, 1, 12, pak->out) != 12) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("Unable to write temporary header for file %s.\n", file);
#endif
      fclose(fp);
      return 0;
    }

    return pak;
  }
}

int32_t pak_append(pak_t* pak, const char* filename, const unsigned char* data,
                   uint32_t data_length) {
  if (!data || !data_length) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_append: invalid data pointer passed.\n");
#endif
    return -1;
  }

  if (!pak->out || !pak->file_mode) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_append: unable to append to read-only pak file.\n");
#endif
    return -1;
  }

  if (pak->file_mode && pak->out) {
    uint32_t index = pak->count++;
    pak->entries   = realloc(pak->entries, pak->count * sizeof(pak_file_t));

    if (!pak->entries) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("pak_append: unable to reallocate entries.\n");
#endif
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
}

int32_t pak_append_file(pak_t* pak, const char* filename, FILE* in) {
  if (!pak) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_append_file: no pak struct passed.\n");
#endif
    return -1;
  }

  if (!filename) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_append_file: no filename passed.\n");
#endif
    return -1;
  }

  if (!in) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_append_file: no in file passed.\n");
#endif
    return -1;
  }

  uint32_t index = pak->count++;

  pak->entries      = realloc(pak->entries, pak->count * sizeof(pak_file_t));
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
#endif

void pak_open_mem(pak_t* pak, unsigned char* data, uint32_t data_length) {
  if (!data || !data_length) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open_mem: invalid data arguments passed.\n");
#endif
    return;
  }

  memset(pak, 0, sizeof(pak_t));

#if !defined(ASTERA_PAK_NO_FILE)
  pak->file_mode = 0;
#endif

  pak_header_t* header = (pak_header_t*)data;
  if (!header) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open_mem: unable to obtain header pointer.\n");
#endif
    return;
  }

  if (memcmp(header->id, "PACK", 4)) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open_mem: invalid pak file.\n");
#endif
    return;
  }

  header->offset = ltoh32(header->offset);
  header->size   = ltoh32(header->size);

  uint32_t num_files = header->size / sizeof(pak_file_t);

  pak_file_t* start = (pak_file_t*)&data[header->offset];
  if (!start) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open_mem: unable to seek to start of data.\n");
#endif
    return;
  }

  pak->count   = num_files;
  pak->entries = (pak_file_t*)malloc(sizeof(pak_file_t) * num_files);

  if (!pak->entries) {
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_open_mem: unable to malloc %i bytes.\n",
       (sizeof(pak_file_t) * num_files));
#endif
    return;
  }

  memset(pak->entries, 0, sizeof(pak_file_t) * num_files);

  for (uint32_t i = 0; i < num_files; ++i) {
    pak_file_t* start_ptr = (pak_file_t*)&start[i];
    pak_file_t* pak_ptr   = (pak_file_t*)&pak->entries[i];

    if (!start_ptr || !pak_ptr) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("pak_open_mem: invalid read of entries.\n");
#endif
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
#if !defined(ASTERA_PAK_NO_FILE)
    if (pak->file_mode && pak->in) {
      if (fseek(pak->in, file->offset, SEEK_SET) != 0) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("File read error: cannot seek to offset %i\n", file->offset);
#endif
        return 0;
      }

      unsigned char* buffer = (unsigned char*)malloc(sizeof(char) * file->size);
      if (!buffer) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("File read error: cannot malloc %i bytes.\n", file->size);
#endif
        return 0;
      }

      if (fread(buffer, 1, file->size, pak->in) != file->size) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("File read error: invalid read size.\n");
#endif
        return 0;
      }

      return buffer;
    } else if (!pak->file_mode && pak->data_ptr) {
#else
    if (pak->data_ptr) {
#endif
      unsigned char* offset = &pak->data_ptr[file->offset];

      if (!offset) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("Memory read error: offset out of bounds: %i\n", file->offset);
#endif
        return 0;
      }

      unsigned char* buffer = (unsigned char*)malloc(sizeof(char) * file->size);

      if (!buffer) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("Memory read error: unable to malloc %i bytes.\n", file->size);
#endif
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
#if defined(ASTERA_DEBUG_OUTPUT)
    _e("pak_extract_noalloc: invalid out buffer parameters passed.\n");
#endif
    return 0;
  }

  if (index < pak->count) {
    pak_file_t* file = &pak->entries[index];

    if (file->size > out_cap) {
#if defined(ASTERA_DEBUG_OUTPUT)
      _e("pak_extract_no_alloc: out_cap too small to hold %i bytes.\n",
         file->size);
#endif
      return 0;
    }

#if !defined(ASTERA_PAK_NO_FILE)
    if (pak->file_mode && pak->in) {
      if (fseek(pak->in, file->offset, SEEK_SET) != 0) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("File read error: cannot seek to offset %i\n", file->offset);
#endif
        return 0;
      }

      if (fread(out, 1, file->size, pak->in) != file->size) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("File read error: invalid read size.\n");
#endif
        return 0;
      }

      return out;
    } else if (!pak->file_mode && pak->data_ptr) {
#else
    if (pak->data_ptr) {
#endif
      unsigned char* offset = &pak->data_ptr[file->offset];

      if (!offset) {
#if defined(ASTERA_DEBUG_OUTPUT)
        _e("Memory read error: offset out of bounds: %i\n", file->offset);
#endif
        return 0;
      }

      memcpy(out, offset, sizeof(char) * file->size);

      return out;
    }
  }
  return 0;
}

pak_t* pak_close(pak_t* pak) {
#if !defined(ASTERA_PAK_NO_FILE)
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
#endif
  if (pak->data_ptr) {
    free(pak->data_ptr);
  }

  if (pak->entries) {
    free(pak->entries);
  }
}

int32_t pak_find(pak_t* pak, const char* filename) {
#if !defined(ASTERA_PAK_NO_FILE)
  if ((pak->file_mode && pak->in) || (!pak->file_mode && pak->data_ptr)) {

    for (uint32_t i = 0; i < pak->count; ++i) {
      if (!strcmp(pak->entries[i].name, filename)) {
        return i;
      }
    }
  }
#else
  if (pak->data_ptr) {
    for (uint32_t i = pak->count; i > 0; --i) {
      if (!strcmp(pak->entries[i].name, filename)) {
        return i;
      }
    }
  }
#endif
  return -1;
}

uint32_t pak_count(pak_t* pak) {
  if (!pak)
    return 0;
#if !defined(ASTERA_PAK_NO_FILE)
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode))
             ? pak->count
             : 0;
#else
  return pak->data_ptr ? pak->count : 0;
#endif
}

uint32_t pak_offset(pak_t* pak, uint32_t index) {
#if !defined(ASTERA_PAK_NO_FILE)
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode)) &&
                 index < pak->count
             ? pak->entries[index].offset
             : 0;
#else
  return pak->data_ptr && index < pak->count ? pak->entries[index].offset : 0;
#endif
}

uint32_t pak_size(pak_t* pak, uint32_t index) {
#if !defined(ASTERA_PAK_NO_FILE)
  return ((pak->in && pak->file_mode) || (pak->data_ptr && !pak->file_mode)) &&
                 index < pak->count
             ? pak->entries[index].size
             : 0;
#else
  return pak->data_ptr && index < pak->count ? pak->entries[index].size : 0;
#endif
}

char* pak_name(pak_t* pak, uint32_t index) {
#if !defined(ASTERA_PAK_NO_FILE)
  return (index < pak->count) && ((pak->file_mode && pak->in) ||
                                  (!pak->file_mode && pak->data_ptr))
             ? pak->entries[index].name
             : 0;
#else
  return pak->data_ptr && index < pak->count ? pak->entries[index].name : 0;
#endif
  return 0;
}

#endif

int8_t asset_init() {
#if ASSET_MAX_MAPS > 0
  memset(asset_maps, 0, sizeof(asset_map_t) * ASSET_MAX_MAPS);
  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    asset_maps[i].free = 1;
  }

  asset_create_map(NULL, "sys", MIN_ASSET_CACHE, 0);
#endif

  return 1;
}

void asset_exit() {
  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    asset_map_free_t(&asset_maps[i]);
  }
}

void asset_free(asset_t* asset) {
  asset->name   = 0;
  asset->filled = 0;
  asset->req    = 0;
  free(asset->data);
}

void asset_map_free(const char* map_name) {
  asset_map_t* map;
  for (uint32_t i = 0; i < asset_map_count; ++i) {
    if (strcmp(asset_maps[i].name, map_name) == 0) {
      map = &asset_maps[i];
      break;
    } else if (strcmp(asset_maps[i].filename, map_name) == 0) {
      map = &asset_maps[i];
      break;
    }
  }

  if (!map) {
    _e("Unable to find map %s to free.\n", map);
    return;
  }

  for (uint32_t i = 0; i < map->capacity; ++i) {
    asset_t* asset = &map->assets[i];
    if (asset->data) {
      free(asset->data);
      asset->data = 0;
    }
  }

  free(map->assets);
  map->assets   = 0;
  map->count    = 0;
  map->capacity = 0;
  map->name     = 0;
  map->filename = 0;
}

void asset_map_free_t(asset_map_t* map) {
  if (!map) {
    _e("No map passed to free.\n");
    return;
  }

  for (uint32_t i = 0; i < map->capacity; ++i) {
    asset_t* asset = &map->assets[i];
    if (asset->data) {
      free(asset->data);
      asset->data = 0;
    }
  }

  free(map->assets);
  map->assets   = 0;
  map->count    = 0;
  map->capacity = 0;
  map->name     = 0;
  map->filename = 0;
}

asset_t* asset_get(const char* map_name, const char* file) {
  asset_map_t* map = NULL;

  if (!file) {
    _e("No file requested.\n");
    return 0;
  }

  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    if (map_name) {
      if (asset_maps[i].name) {
        if (!strcmp(asset_maps[i].name, map_name)) {
          map = &asset_maps[i];
          break;
        }
      }

      if (asset_maps[i].filename) {
        if (!strcmp(asset_maps[i].filename, map_name)) {
          map = &asset_maps[i];
          break;
        }
      }
    } else {
      if (asset_maps[i].name) {
        if (!strcmp(asset_maps[i].name, "sys")) {
          map = &asset_maps[i];
          break;
        }
      }
    }
  }

  if (!map) {
    _l("No map with name or filepath of: %s\n", map_name);
    return 0;
  }

  asset_t* asset = NULL;
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!asset) {
      if ((!map->assets[i].filled && !map->assets[i].req) ||
          map->assets[i].req_free) {
        asset = &map->assets[i];
      }
    }

    if (map->assets[i].name) {
      if (strcmp(map->assets[i].name, file) == 0) {
        asset = &map->assets[i];

        // Don't double read
        if (map->assets[i].filled)
          return &map->assets[i];
      }
    }
  }

  // At capacity for asset map
  if (!asset) {
    _e("No free asset slots in map %s\n", map->name);
    return 0;
  }

  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!map->assets[i].filled && !map->assets[i].req) {
      asset = &map->assets[i];
      break;
    }
  }

  if (strcmp(map->name, "sys") == 0) {
    FILE* f = fopen(file, "r+b");

    if (!f) {
      _e("Unable to open system file: %s\n", file);
      return NULL;
    }

    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);
    rewind(f);

    unsigned char* data =
        (unsigned char*)malloc(sizeof(unsigned char) * (file_size + 1));

    if (!data) {
      _e("Unable to allocate %i bytes for file %s\n", file_size, file);
      fclose(f);
      return NULL;
    }

    uint32_t data_read = fread(data, sizeof(unsigned char), file_size, f);

    if (data_read != file_size) {
      _l("Incomplete read: %i expeceted, %i read.\n", file_size, data_read);
    }

    data[data_read] = 0;

    fclose(f);
    asset->data        = data;
    asset->data_length = data_read;
  } else {
#if !defined(ASTERA_NO_ZIP)
    struct zip_t* zip;

    zip = zip_open(map->filename, map->compression_level, 'r');
    if (!zip) {
      _e("Unable to open zip entry: %s.\n", map->filename);
      return NULL;
    }

    if (zip_entry_open(zip, file) < 0) {
      _e("Unable to open file: %s in zip archive: %s\n", file, map->filename);
      zip_close(zip);
      return NULL;
    }

    uint32_t       entry_size = zip_entry_size(zip);
    unsigned char* data =
        (unsigned char*)malloc(sizeof(unsigned char) * entry_size);
    if (!data) {
      _e("Unable to allocate %i bytes for file %s.\n", entry_size, file);
      zip_close(zip);
      return NULL;
    }

    // TODO more error handling
    zip_entry_noallocread(zip, (void*)data, entry_size);

    zip_close(zip);

    asset->data        = data;
    asset->data_length = entry_size;
#endif
  }

  asset->name     = file;
  asset->filled   = 1;
  asset->req      = 0;
  asset->req_free = 0;
  asset->chunk    = 0;

  ++asset_uid_count;
  asset->uid = asset_uid_count;

  return asset;
}

asset_t* asset_req(const char* map_name, const char* file) {
  asset_map_t* map = 0;
  for (uint32_t i = 0; i < asset_map_count; ++i) {
    if (map_name) {
      if (asset_maps[i].name) {
        if (strcmp(asset_maps[i].name, map_name) == 0) {
          map = &asset_maps[i];
          break;
        }
      }

      if (asset_maps[i].filename) {
        if (strcmp(asset_maps[i].filename, map_name) == 0) {
          map = &asset_maps[i];
          break;
        }
      }
    } else {
      if (!strcmp(asset_maps[i].name, "sys")) {
        map = &asset_maps[i];
        break;
      }
    }
  }

  if (map_name) {
    if (!map) {
      _l("No map with name or filepath of: %s\n", map_name);
      return 0;
    }
  } else {
    _l("No sys map exists.\n");
    return 0;
  }

  asset_t* asset = 0;
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!map->assets[i].filled && !map->assets[i].req) {
      asset = &map->assets[i];
    }

    if (strcmp(map->assets[i].name, file) == 0) {
      if (!map->assets[i].req) {
        map->assets[i].req   = 1;
        map->assets[i].chunk = 0;
      }

      return &map->assets[i];
    }
  }

  if (!asset)
    return 0;

  asset->req    = 1;
  asset->chunk  = 0;
  asset->filled = 0;
  asset->name   = file;
  asset->uid    = ++asset_uid_count;

  return asset;
}

asset_t* asset_get_chunk(const char* map_name, const char* file,
                         uint32_t chunk_start, uint32_t chunk_length) {
  if (!file) {
    _e("No file requested\n");
    return 0;
  }

  asset_map_t* map = NULL;

  if (!file) {
    return 0;
  }

  if (!map_name) {
    map_name = "sys";
  }

  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    if (map_name) {
      if (asset_maps[i].name) {
        if (strcmp(asset_maps[i].name, map_name) == 0) {
          map = &asset_maps[i];
          break;
        }
      }

      if (asset_maps[i].filename) {
        if (strcmp(asset_maps[i].filename, map_name) == 0) {
          map = &asset_maps[i];
          break;
        }
      }
    } else {
      if (asset_maps[i].name) {
        if (!strcmp("sys", asset_maps[i].name)) {
          map = &asset_maps[i];
          break;
        }
      }
    }
  }

  if (!map) {
    _l("No map with name or filepath of: %s\n", map_name);
    return 0;
  }

  asset_t* asset = NULL;

  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!asset) {
      if (!map->assets[i].filled && !map->assets[i].req) {
        asset = &map->assets[i];
        continue;
      }
    }

    if (map->assets[i].name) {
      if (strcmp(map->assets[i].name, file) == 0) {
        asset = &map->assets[i];
        break;
      }
    }
  }

  if (!asset) {
    _e("No space in map %s for file %s\n", map_name, file);
    return 0;
  }

  if (strcmp(map->name, "sys") == 0) {
    FILE* f = fopen(file, "r+b");

    if (!f) {
      _e("Unable to open system file: %s\n", file);
      return NULL;
    }

    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);
    rewind(f);

    if (chunk_start > file_size) {
      _e("Chunk requested starts out of bounds [%i] of the file [%i].\n",
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
      _e("Unable to allocate [%i] bytes for file chunk %s\n", max_length, file);
      fclose(f);
      return NULL;
    }

    uint32_t data_read = fread(data, sizeof(unsigned char), max_length, f);

    if (data_read != max_length) {
      _l("Incomplete read: %i expeceted, %i read.\n", max_length, data_read);
    }

    data[data_read] = 0;

    fclose(f);

    asset->data_length = data_read;
    asset->data        = data;
  } else {
#if !defined(ASTERA_NO_ZIP)
    struct zip_t* zip;

    zip = zip_open(map->filename, map->compression_level, 'r');
    if (!zip) {
      _e("Unable to open zip entry: %s.\n", map->filename);
      return NULL;
    }

    if (zip_entry_open(zip, file) < 0) {
      _e("Unable to open file: %s in zip archive: %s\n", file, map->filename);
      zip_close(zip);
      asset->req   = 0;
      asset->chunk = 0;
      return NULL;
    }

    uint32_t entry_size = zip_entry_size(zip);

    if (chunk_start > entry_size) {
      zip_close(zip);
      asset->req   = 0;
      asset->chunk = 0;
      return 0;
    }

    unsigned char* data =
        (unsigned char*)malloc(sizeof(unsigned char) * entry_size);
    if (!data) {
      _e("Unable to allocate %i bytes for file %s.\n", entry_size, file);
      zip_close(zip);
      return 0;
    }

    uint32_t max_length = (chunk_start + chunk_length > entry_size)
                              ? entry_size - chunk_start
                              : chunk_length;

    unsigned char* chunk =
        (unsigned char*)malloc(sizeof(unsigned char) * (max_length + 1));

    if (!chunk) {
      _e("Unable to allocate [%i] bytes for chunk of file [%s]\n", max_length,
         file);
      free(data);
      zip_close(zip);
      return 0;
    }

    memcpy(chunk, &data[chunk_start], max_length);
    chunk[max_length] = '\0';

    free(data);

    zip_entry_noallocread(zip, (void*)data, entry_size);

    zip_close(zip);

    asset->data         = chunk;
    asset->data_length  = max_length;
    asset->chunk_length = max_length;
#endif
  }

  asset->name        = file;
  asset->filled      = 1;
  asset->chunk       = 1;
  asset->chunk_start = chunk_start;
  asset->req         = 0;
  asset->req_free    = 0;

  asset->uid = ++asset_uid_count;

  return asset;
}

asset_t* asset_req_chunk(const char* map_name, const char* file,
                         uint32_t chunk_start, uint32_t chunk_length) {
  if (!file) {
    _e("No file requested.\n");
    return 0;
  }

  asset_map_t* map = 0;
  for (uint32_t i = 0; i < asset_map_count; ++i) {
    if (map_name) {
      if (strcmp(asset_maps[i].name, map_name) == 0) {
        map = &asset_maps[i];
        break;
      } else if (strcmp(asset_maps[i].filename, map_name) == 0) {
        map = &asset_maps[i];
        break;
      }
    } else {
      if (strcmp(asset_maps[i].name, "sys") == 0) {
        map = &asset_maps[i];
        break;
      }
    }
  }

  if (!map) {
    _l("No map with name or filepath of [%s]\n", map_name);
    return 0;
  }

  asset_t* asset = 0;
  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (!asset) {
      if (!map->assets[i].filled && !map->assets[i].req) {
        asset = &map->assets[i];
      }
    }

    if (strcmp(map->assets[i].name, file) == 0) {
      if (!map->assets[i].req && !map->assets[i].filled) {
        map->assets[i].req          = 1;
        map->assets[i].chunk        = 1;
        map->assets[i].chunk_start  = chunk_start;
        map->assets[i].chunk_length = chunk_length;

        return &map->assets[i];
      } else if (map->assets[i].filled) {
        if (map->assets[i].chunk && map->assets[i].chunk_start == chunk_start &&
            map->assets[i].chunk_length == chunk_length) {
          return &map->assets[i];
        }
      }
    }
  }

  if (!asset) {
    _e("No free slots in asset map: %s.\n", map_name);
    return 0;
  }

  asset->req = 1;

  asset->chunk        = 1;
  asset->chunk_start  = chunk_start;
  asset->chunk_length = chunk_length;

  asset->filled = 0;
  asset->name   = file;
  asset->uid    = ++asset_uid_count;

  return asset;
}

asset_map_t* asset_create_map(const char* filename, const char* name,
                              uint32_t capacity, uint8_t compression_level) {
  uint32_t free_maps = 0;
  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    if (asset_maps[i].free) {
      ++free_maps;
      continue;
    }

    if (filename) {
      if (strcmp(filename, asset_maps[i].filename) == 0) {
        return &asset_maps[i];
      }
    }

    if (strcmp(name, asset_maps[i].name) == 0) {
      return &asset_maps[i];
    }
  }

  if (!free_maps) {
    return 0;
  }

  for (uint32_t i = 0; i < ASSET_MAX_MAPS; ++i) {
    if (asset_maps[i].free) {
      asset_maps[i].assets = (asset_t*)malloc(sizeof(asset_t) * capacity);
      asset_maps[i].compression_level = compression_level;
      asset_maps[i].capacity          = capacity;
      asset_maps[i].filename          = filename;
      asset_maps[i].name              = name;
      asset_maps[i].free              = 0;
      return &asset_maps[i];
    }
  }

  return 0;
}

void asset_update_map(asset_map_t* map) {
#if !defined(ASTERA_NO_ZIP)
  struct zip_t* zip;
#endif

  for (uint32_t i = 0; i < map->capacity; ++i) {
    if (map->assets[i].req_free) {
      free(map->assets[i].data);
      memset(&map->assets[i], 0, sizeof(asset_t));
    }

#if !defined(ASTERA_NO_ZIP)
    if (map->assets[i].req) {
      if (!zip)
        zip = zip_open(map->filename, map->compression_level, 'r');
      if (!zip) {
        _e("Unable to open: %s\n", map->filename);
        break;
      }

      if (zip_entry_open(zip, map->assets[i].name) < 0) {
        _e("Unable to open file: %s in archive: %s\n", map->assets[i].name,
           map->filename);
        zip_entry_close(zip);
        continue;
      }

      uint32_t file_size = zip_entry_size(zip);
      if (!file_size) {
        _e("Unable to open file with size of 0, %s.\n", map->assets[i].name);
        continue;
      }

      map->assets[i].data =
          (unsigned char*)malloc(file_size * sizeof(unsigned char));
      map->assets[i].data_length = file_size;
      map->assets[i].req         = 0;
      map->assets[i].filled      = 1;

      zip_entry_noallocread(zip, (void*)map->assets[i].data, file_size);
      zip_entry_close(zip);
      ++map->count;
    }
#endif
  }

#if !defined(ASTERA_NO_ZIP)
  if (zip)
    zip_close(zip);
#endif
}
