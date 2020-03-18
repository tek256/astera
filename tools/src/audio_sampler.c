#include <libgen.h>
#include <stb_vorbis.c>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char *get_data(const char *file_path, int *file_length) {
  FILE *f = fopen(file_path, "r+");
  if (!f) {
    printf("Unable to open file: %s\n", file_path);
    return 0;
  }

  fseek(f, 0, SEEK_END);
  int length = ftell(f);
  rewind(f);

  unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) * length);
  int data_read = fread(data, length, sizeof(unsigned char), f);
  fclose(f);

  if (data_read != length) {
    printf("Unable to read full length of file: %s.\n [%i] bytes requested, "
           "[%i] bytes received.\n",
           file_path, length, data_read);
    free(data);
    return 0;
  }

  if (file_length) {
    *file_length = length;
  }

  return data;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Arguments are required.\n");
    printf("Pass arguments as: (audio_file) name,offset name,offset ... n.\n");
    return EXIT_FAILURE;
  }

  if (strcmp(argv[1], "--h") == 0 || strcmp(argv[1], "--help") == 0 ||
      strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
    printf("Pass arguments as: (audio_file) name,offset name,offset ... n.\n");
    return EXIT_SUCCESS;
  }

  char *file_path = argv[1];

  if (!file_path) {
    printf("We need an audio file path\n");
    return EXIT_FAILURE;
  }

  char *names[argc - 2];
  float keyframes[argc - 2];
  int keyframe_count = 0;

  for (int i = 2; i < argc; ++i) {
    char tok[2] = ",";
    char *index = strtok(argv[i], tok);
    int values = 0;

    while (index != NULL) {
      if (values == 0) {
        names[keyframe_count] = index;
      } else if (values == 1) {
        keyframes[keyframe_count] = atof(index);
      }

      ++values;
      index = strtok(NULL, tok);
    }
    ++keyframe_count;
  }

  for (int i = 0; i < keyframe_count; ++i) {
    printf("%s [%.2f]\n", names[i], keyframes[i]);
  }

  FILE *f = fopen(file_path, "r+");
  if (!f) {
    printf("Unable to open file: %s\n", file_path);
    return EXIT_FAILURE;
  }

  fseek(f, 0, SEEK_END);
  int length = ftell(f);
  rewind(f);

  unsigned char *data = (unsigned char *)malloc(sizeof(char) * length);
  int data_read = fread(data, sizeof(unsigned char), length, f);

  if (data_read != length) {
    printf("Partial read: %i requested %i received.\n", length, data_read);
    free(data);
    return EXIT_FAILURE;
  }

  fclose(f);

  int error;
  stb_vorbis *vorbis = stb_vorbis_open_memory(data, length, &error, NULL);

  if (!vorbis) {
    free(data);
    printf("Unable to open memory.\n");
    return EXIT_FAILURE;
  }

  int num_channels, sample_rate;
  short *output;
  int sample_count = stb_vorbis_decode_memory(data, length, &num_channels,
                                              &sample_rate, &output);

  if (sample_count == 0) {
    printf("Unable to load samples from: %i\n", length);
    free(data);
    return EXIT_FAILURE;
  }

  printf("%i sample rate.\n", sample_rate);
  printf("%i samples parsed.\n", sample_count);
  printf("%i channels.\n", num_channels);

  float time_seconds = sample_count / sample_rate;
  printf("time in seconds: %f\n", time_seconds);

  int total_minutes = time_seconds / 60;
  int total_seconds = (total_minutes * 60) - time_seconds;

  printf("Generating keyframes now.\n");

  char name[128];
  char line[128];

  strcpy(name, file_path);
  strcat(name, ".mta");

  FILE *out = fopen(name, "w+");
  int num_chars;

  num_chars = sprintf(line, "sample_rate,%i\n", sample_rate);
  fwrite(line, sizeof(char), num_chars, out);
  memset(line, 0, 128 * sizeof(char));

  num_chars = sprintf(line, "num_channels,%i\n", num_channels);
  fwrite(line, sizeof(char), num_chars, out);
  memset(line, 0, 128 * sizeof(char));

  num_chars = sprintf(line, "start,0\n");
  fwrite(line, sizeof(char), num_chars, out);
  memset(line, 0, 128 * sizeof(char));

  num_chars = sprintf(line, "end,%i\n", sample_count);
  fwrite(line, sizeof(char), num_chars, out);
  memset(line, 0, 128 * sizeof(char));

  for (int i = 0; i < keyframe_count; ++i) {
    float keyframe = keyframes[i];
    printf("Reconfiguring keyframe: %s, %f\n", names[i], keyframe);

    int minutes = (int)keyframe;
    float seconds = ((keyframe - minutes) * 100.f) / 60.f;

    printf("Seconds: %f\n", seconds);

    int est_sample = (sample_rate * minutes) + (sample_rate * (seconds));

    printf("Estimated sample: %i\n", est_sample);

    num_chars = sprintf(line, "%s,%i\n", names[i], est_sample);
    fwrite(line, sizeof(char), num_chars, out);
    memset(line, 0, sizeof(char) * 128);
  }

  fclose(out);
  free(data);

  return EXIT_SUCCESS;
}
