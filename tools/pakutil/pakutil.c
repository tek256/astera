// Use this for all your pak needs
// usage:
// pakutil [(m)ake|(c)heck|(d)ata] pak_file_path file ... file n

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <astera/asset.h>

typedef enum {
  NONE = 0,
  MAKE,
  CHECK,
  DATA,
} usage_modes;

int main(int argc, char** argv) {
#if defined(ASTERA_PAK_WRITE)
  if (argc == 1) {
    printf("Usage: ./pakutil [(m)ake|(c)heck|(d)ata] dst.pak "
           "file ... "
           "file n\n");
    return 0;
  } else if (argc == 2) {
    if (strcmp(argv[1], "h") == 0 || strcmp(argv[1], "help") == 0 ||
        strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0 ||
        strcmp(argv[1], "--h") == 0 || strcmp(argv[1], "--help") == 0) {
      printf("Usage: ./pakutil [(m)ake|(c)heck|(d)ata] dst.pak "
             "file ... "
             "file n\n");
      return 0;
    }
  } else if (argc == 3) {
    if (!strcmp(argv[1], "h") || !strcmp(argv[1], "help") ||
        !strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") ||
        !strcmp(argv[1], "--h") || !strcmp(argv[1], "--help")) {
      if (!strcmp(argv[2], "m") || !strcmp(argv[2], "make")) {
        printf("Pak Util Make: Create a pak file\n");
        printf("Usage: ./pakutil make dst.pak filepath name ... "
               "filepath n file name n\n");
        printf("Ex: ./pakutil make example.pak resources/shaders/main.vert "
               "main.vert resources/shaders/main.frag main.frag\n");
        return 0;
      } else if (!strcmp(argv[2], "c") || !strcmp(argv[2], "check")) {
        printf("Pak Util Check: Check a pak file for file(s)\n");
        printf("Usage: ./pakutil check dst.pak (list out all files)OR\n"
               "./pakutil check dst.pak filename ... filename n\n");
        printf("Ex 1: ./pakutil check example.pak\n");
        printf("Ex 2: ./pakutil check example.pak main.frag\n");
        return 0;
      } else if (!strcmp(argv[2], "d") || !strcmp(argv[2], "data")) {
        printf("Pak Util Data: Get the raw data of file(s) in the pak file\n");
        printf("Usage: ./pakutil data dst.pak filename .. filename "
               "n\n");
        printf("Ex: ./pakutil data example.pak test2.txt main.vert\n");
        return 0;
      }
    }
  }

  int mode = NONE;

  if (strcmp(argv[1], "make") == 0 || strcmp(argv[0], "m") == 0) {
    mode = MAKE;
  } else if (strcmp(argv[1], "check") == 0 || strcmp(argv[0], "c") == 0) {
    mode = CHECK;
  } else if (strcmp(argv[1], "data") == 0 || strcmp(argv[0], "d") == 0) {
    mode = DATA;
  }

  if (mode == NONE) {
    printf("Invalid mode passed\n");
    return 0;
  }

  const char* pak_file = argv[2];
  pak_t*      pak      = 0;

  switch (mode) {
    case MAKE: {
      pak_write_t* write = pak_write_create(pak_file);

      if (!write) {
        return 1;
      }

      for (int i = 3; i < argc - 1; i += 2) {
        const char* name = argv[i + 1];
        const char* fp   = argv[i];
        printf("%s\n", fp);
        if (!pak_write_add_file(write, fp, fp)) {
          printf("Unable to add file: %s\n", fp);
        }
      }

      pak_write_to_file(write);

      pak_write_destroy(write);
    } break;
    case CHECK: {
      pak = pak_open_file(pak_file);

      if (!pak) {
        return 1;
      }

      int count = pak_count(pak);
      printf("%s contains: %i files\n", argv[2], count);

      if (argc == 3) {
        for (int i = 0; i < pak_count(pak); ++i) {
          printf("%s index: [%i] size: [%i] offset: [%i]\n", pak_name(pak, i),
                 i, pak_size(pak, i), pak_offset(pak, i));
        }
      } else {
        for (int i = 3; i < argc; ++i) {
          int32_t find = pak_find(pak, argv[i]);

          if (find > -1) {
            printf("%s index: [%i] size: [%i] offset: [%i]\n", argv[i], find,
                   pak_size(pak, find), pak_offset(pak, find));
          } else {
            printf("No match for %s\n", argv[i]);
          }
        }
      }
    } break;
    case DATA: {
      pak = pak_open_file(pak_file);

      if (!pak) {
        printf("Unable to open pak file.\n");
        return 1;
      }

      for (int i = 3; i < argc; ++i) {
        int find = pak_find(pak, argv[i]);
        if (find != -1) {
          unsigned char* data = pak_extract(pak, find, 0);
          printf("%s", data);
          free(data);
        }
      }

    } break;
    default:
      printf("How did you get here?");
      break;
  }

  if (pak)
    pak_close(pak);

  return 0;
#else
  printf("Please enable ASTERA_PAK_WRITE in the build system to get this "
         "program, thank you!\n");
  return 0;
#endif
}
