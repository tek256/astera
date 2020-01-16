#include "conf.h"

#include "platform.h"

#include "debug.h"
#include "input.h"

#include <getopt.h>
#include <unistd.h>

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static c_args _flags;

c_conf c_defaults() {
  return (c_conf){1280, 720, 0, 60, 1, 0, 100, 75, 50, 2.2f, NULL, NULL};
}

void c_parse_args(int argc, char **argv) {
  if (!argv || !argc) {
    return;
  }

  _flags = (c_args){0, 1, 1, 0, NULL};

  while (1) {
    char c = getopt(argc, argv, "srvadc:");

    if (c == -1) {
      break;
    }

    switch (c) {
    case 's':
      _flags.verbose = -1;
      break;
    case 'r':
      _flags.render = 0;
      break;
    case 'a':
      _flags.audio = 0;
      break;
    case 'v':
      _flags.verbose = 1;
      break;
    case 'd':
      _flags.debug = 1;
      break;
    case 'c': {
      char max_conf[64];
      snprintf(max_conf, 63, "%s", optarg);
      int opt_len = strlen(max_conf);
      _flags.prefs = malloc(sizeof(char) * opt_len);
      strcpy(_flags.prefs, max_conf);
    } break;
    }
  }
}

// Quality of life thing, can factor out later
#define STR_MATCH(s, a) strcmp(s, a) == 0

c_conf c_parse_table(c_table table) {
  c_conf default_conf = c_defaults();
  c_conf out_conf = default_conf;

  for (int i = 0; i < table.count; ++i) {
    if (out_conf.width == default_conf.width) {
      if (STR_MATCH(table.keys[i], "width")) {
        out_conf.width = atoi(table.values[i]);
      }
    }

    if (out_conf.height == default_conf.height) {
      if (STR_MATCH(table.keys[i], "height")) {
        out_conf.height = atoi(table.values[i]);
      }
    }

    if (out_conf.fullscreen == default_conf.fullscreen) {
      if (STR_MATCH(table.keys[i], "fullscreen")) {
        out_conf.fullscreen = atoi(table.values[i]);
      }
    }

    if (out_conf.refreshRate == default_conf.refreshRate) {
      if (STR_MATCH(table.keys[i], "refreshRate")) {
        out_conf.refreshRate = atoi(table.values[i]);
      }
    }

    if (out_conf.vsync == default_conf.vsync) {
      if (STR_MATCH(table.keys[i], "vsync")) {
        out_conf.vsync = atoi(table.values[i]);
      }
    }

    if (out_conf.borderless == default_conf.borderless) {
      if (STR_MATCH(table.keys[i], "borderless")) {
        out_conf.borderless = atoi(table.values[i]);
      }
    }

    if (out_conf.master == default_conf.master) {
      if (STR_MATCH(table.keys[i], "master")) {
        out_conf.master = atoi(table.values[i]);
      }
    }

    if (out_conf.music == default_conf.music) {
      if (STR_MATCH(table.keys[i], "music")) {
        out_conf.music = atoi(table.values[i]);
      }
    }

    if (out_conf.sfx == default_conf.sfx) {
      if (STR_MATCH(table.keys[i], "sfx")) {
        out_conf.sfx = atoi(table.values[i]);
      }
    }
    if (out_conf.icon == default_conf.icon) {
      if (STR_MATCH(table.keys[i], "icon")) {
        out_conf.icon = strdup(table.values[i]);
      }
    }

    if (out_conf.gamma == default_conf.gamma) {
      if (STR_MATCH(table.keys[i], "gamma")) {
        out_conf.gamma = atof(table.values[i]);
      }
    }
  }

  return out_conf;
}

void c_write_pref(const char *fp, const char *key, const char *value) {}

void c_table_free(c_table table) {
  if (table.keys)
    free(table.keys);
  if (table.values)
    free(table.values);
}

static char *c_cleaned_str(char *str) {
  char *end;
  while (isspace(*str))
    str++;

  if (*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace(*end))
    end--;
  end[1] = '\0';

  return str;
}

c_table c_get_table(asset_t *asset) {
  assert(asset->filled);
  assert(asset->data);
  char *raw_ptr = (char *)asset->data;
  char *tok;
  tok = strtok_r(raw_ptr, "\n", &raw_ptr);

  char *raw_data = malloc(sizeof(char) * 512);
  int raw_capacity = 512;
  int raw_count = 0;

  const char **keys = malloc(sizeof(char *) * 16);
  const char **values = malloc(sizeof(char *) * 16);
  int line_capacity = 16;
  int line_count = 0;

  while (tok) {
    char *line = tok;
    strtok(line, "=");

    char *key, *value;

    int parts = 0;
    int key_length = 0;
    int value_length = 0;
    while (line) {
      line = c_cleaned_str(line);
      if (!parts) {
        key_length = strlen(line) + 1;
        key = line;
        ++parts;
      } else if (parts) {
        value_length = strlen(line) + 1;
        value = line;

        if (line_count == line_capacity) {
          keys = realloc(keys, sizeof(char *) * (line_capacity + 8));
          values = realloc(values, sizeof(char *) * (line_capacity + 8));
          line_capacity += 8;
        }

        if (raw_count + (key_length + value_length) > raw_capacity) {
          int difference =
              (64 + raw_count + key_length + value_length) - raw_capacity;
          raw_data =
              realloc(raw_data, sizeof(char) * (raw_capacity + difference));
          raw_capacity += difference;
        }

        keys[line_count] = strcpy(&raw_data[raw_count], key);
        raw_count += key_length;

        values[line_count] = strcpy(&raw_data[raw_count], value);
        raw_count += value_length;

        ++line_count;
      }

      line = strtok(NULL, "=");
    }

    tok = strtok_r(NULL, "\n", &raw_ptr);
  }

  /*for (int i = 0; i < line_count; ++i) {
    printf("%s:%s\n", keys[i], values[i]);
  }

  printf("%i lines\n", line_count);*/

  return (c_table){keys, values, line_count};
}

int c_has_prefs() { return _flags.prefs != 0; }

char *c_get_pref_p() { return _flags.prefs; }

int c_is_debug() { return _flags.debug; }

int c_allow_render() { return _flags.render; }

int c_allow_audio() { return _flags.audio; }

int c_is_silent() { return _flags.verbose == -1; }

int c_is_verbose() { return _flags.verbose; }
