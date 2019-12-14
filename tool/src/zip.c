#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <zip/zip.h>

#define ZIP_ARCHIVE "test.zip"

unsigned char *get_data(const char *file_path, int *data_size) {
  FILE *f = fopen(file_path, "r+");
  if (!f) {
    printf("Unable to open: %s\n", file_path);
    return NULL;
  }

  fseek(f, 0, SEEK_END);
  int file_size = ftell(f);
  rewind(f);

  unsigned char *data = malloc(sizeof(unsigned char) * file_size);
  int data_read = fread(data, sizeof(unsigned char), file_size, f);

  if (data_read != file_size) {
    printf("The file is [%i] bytes long but we read [%i] bytes.\n", file_size,
           data_read);
  }

  fclose(f);

  if (data_size) {
    *data_size = data_read;
  }

  return data;
}

int main(int argc, char **argv) {
  remove(ZIP_ARCHIVE);
  struct zip_t *zip = zip_open(ZIP_ARCHIVE, 0, 'w');

  for (int i = 1; i < argc; ++i) {
    int size;
    unsigned char *data = get_data(argv[i], &size);
    printf("basename: %s original: %s\n", basename(argv[i]), argv[i]);
    zip_entry_open(zip, basename(argv[i]));
    zip_entry_write(zip, data, size);
    zip_entry_close(zip);
  }

  zip_close(zip);

  return 0;
}
