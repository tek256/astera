/* ASSETS EXAMPLE (Asset)

This example is meant to show how to use the various asset functions in Astera.

If ASTERA_PAK_WRITE is enabled in the build it will include a pak writing
example as well. _NOTE: You can enable this by setting it in your CMakeLists.txt
or passing the argument `-DASTERA_PAK_WRITE=ON` initially._ This is meant to
show how to use the Astera pak writing utility to store multiple files into
astera's .PAK file type. For more on the specifics of this type, check out the
Asset Guide's PAK Files section.

NOTE:
If you want to recreate the pak functionality outside of runtime, astera
includes a pakutil tool in the tools/ directory. You can enable the compilation
of this with the engine by passing -DASTERA_BUILD_TOOLS=ON or setting
ASTERA_BUILD_TOOLS to ON in your CMakeLists.txt. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <astera/asset.h>

void test_load(const char* asset_path) {
  printf("%s:\n", asset_path);

  asset_t* asset = asset_get(asset_path);
  if (!asset) {
    printf("Unable to locate file: %s\n", asset_path);
    return;
  }
  if (asset->filled) {
    printf("Name: %s\nSize: %i\n", asset->name, asset->data_length);
    // astera_hash hash = asset_hash(asset);
    // printf("Hash: %016llx\n", hash);
    printf("OK\n");
    asset_free(asset);
  } else {
    printf("FAIL\n");
  }
  printf("\n");
}

int main() {
  printf("--- Asset Test ---\n\n");

  test_load("resources/textures/tilemap.png");
  test_load("resources/textures/icon.png");
  test_load("resources/fonts/OpenSans-Regular.ttf");
  test_load("resources/shaders/fbo.vert");
  test_load("resources/shaders/fbo.frag");

  // NOTE: To enable ASTERA_PAK_WRITE globally, make sure ASTERA_PAK_WRITE is
  //        enabled at build time (CMake)
#if defined(ASTERA_PAK_WRITE)

  printf("-- PAK Write Test --\n");

  // Create the pak write structure
  pak_write_t* write = pak_write_create("test.pak");

  // Add the string data as a file to the pak file
  const char test_str[] = "THIS IS A TEST";
  pak_write_add_mem(write, test_str, sizeof(test_str), "test2.txt");

  // Add some existing files
  pak_write_add_file(write, "resources/shaders/fbo.vert", "fbo.vert");
  pak_write_add_file(write, "resources/shaders/fbo.frag", "fbo.frag");

  // write out the pak file now
  pak_write_to_file(write);

  // clear up any of the stuff for writing
  pak_write_destroy(write);

  // Open up a read only version of the pak file just written
  pak_t* pak = pak_open_file("test.pak");

  printf("# of files in PAK: %i\n\n", pak_count(pak));

  uint32_t size = pak_size(pak, 1) + 16;

  printf("Index : Name\n");
  for (uint32_t i = 0; i < pak_count(pak); ++i) {
    printf("%5i : %s\n", i, pak_name(pak, i));
  }
  printf("\n");

  // Get the data from the 1st indexed file
  unsigned char* data = pak_extract(pak, 0, 0);

  if (data) {
    printf("Data of file %s:\n", pak_name(pak, 0));
    printf("%s\n", data);
    free(data);
  }

  // Convert the pak's checksum to string
  // char     checksum[32] = {0};
  // uint32_t length       = asset_hash_to_str(pak->checksum, checksum, 32);

  // Output the checksum
  // printf("\nHeader Checksum: %s\n", checksum);

  // Close out the pak file as we no longer need it
  pak_close(pak);

#endif

  return 0;
}
