/* DEBUG EXAMPLE
    This example is meant to show how to use the debug system within astera
*/

#include <astera/debug.h>
#include <time.h>

// add "YES:" to the start of debug outputs
char* log_prepend(void) {
  return "YES: ";
}

int main(void) {
  // enable timestampping in the log outputs
  d_use_timestamp(1);
  // Hour : Minutes format for timestamps
  d_set_timestamp_fmt("[%H:%M]");

  // enable file output
  d_set_log(1, "debug_output.txt");

  // output "Yes this is an output before!\n"
  _l("Yes this is an output before!\n");

  // add the prepend function to add before output lines
  d_set_format_func(log_prepend);

  // output again now with the prepend function
  _l("Hello yes!\n");

  // remove the prepend function
  d_set_format_func(0);

  // test output again!
  _l("Test me here!\n");
  return 0;
}
