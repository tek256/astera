// TODO write_pref
// TODO refactor to user.h (naming collision with col.h)
#ifndef ASTERA_CONF_HEADER
#define ASTERA_CONF_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include <astera/export.h>

typedef struct {
  int   render;
  int   audio;
  int   debug;
  int   verbose;
  char* prefs;
} c_args;

typedef struct {
  const char** keys;
  const char** values;
  int          count;
} c_table;

typedef struct {
  unsigned int width, height;
  unsigned int fullscreen, refreshRate;
  unsigned int vsync, borderless;
  unsigned int master;
  unsigned int music, sfx;
  float        gamma;
  char*        icon;
  char*        path;
} c_conf;

ASTERA_API void    c_parse_args(int argc, char** argv);
ASTERA_API c_conf  c_defaults();
ASTERA_API c_conf  c_parse_table(c_table table);
ASTERA_API c_table c_get_table(unsigned char* data, int length);
ASTERA_API void    c_table_free(c_table table);

ASTERA_API void c_write_pref(const char* fp, const char* key,
                             const char* value);

ASTERA_API int   c_has_prefs();
ASTERA_API char* c_get_pref_p();
ASTERA_API int   c_is_debug();
ASTERA_API int   c_allow_render();
ASTERA_API int   c_allow_audio();
ASTERA_API int   c_is_silent();
ASTERA_API int   c_is_verbose();

#ifdef __cplusplus
}
#endif
#endif
