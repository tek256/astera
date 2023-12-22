/* CONFIG EXAMPLE: Using my terribly built ini reader/writer */

#include <astera/asset.h>
#include <astera/debug.h>
#include <astera/sys.h>
#include <time.h>

// add "INI:" to the start of debug outputs
char* log_prepend(void) {
  return "INI: ";
}

int main(void) {
  // add the prepend function to add before output lines
  d_set_format_func(log_prepend);

  s_table table = s_table_create(256, 32, 1);
  s_table_add_int(&table, "souls", 42);

  // Write to file
  s_table_write(&table, "test.ini");
  // Free in memory copy
  s_table_free(&table);

  // Get the file data
  asset_t* table_data = asset_get("test.ini");

  // Read file
  table = s_table_get(table_data->data);

  _l("souls: %s\n", s_table_find(&table, "souls"));

  s_table_free(&table);
  asset_free(table_data);

  return 0;
}
