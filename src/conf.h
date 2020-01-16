#ifndef CONF_H
#define CONF_H

#include "asset.h"

typedef struct {
  int render;
  int audio;
  int debug;
  int verbose;
  char *prefs;
} c_args;

typedef struct {
  const char **keys;
  const char **values;
  int count;
} c_table;

typedef struct {
  unsigned int width, height;
  unsigned int fullscreen, refreshRate;
  unsigned int vsync, borderless;
  unsigned int master;
  unsigned int music, sfx;
  float gamma;
  char *icon;
  char *path;
} c_conf;

void c_parse_args(int argc, char **argv);
c_conf c_defaults();
c_conf c_parse_table(c_table table);
c_table c_get_table(asset_t *asset);
void c_table_free(c_table table);

void c_write_pref(const char *fp, const char *key, const char *value);

int c_has_prefs();
char *c_get_pref_p();
int c_is_debug();
int c_allow_render();
int c_allow_audio();
int c_is_silent();
int c_is_verbose();

#endif
