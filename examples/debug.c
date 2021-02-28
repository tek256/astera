#include <astera/debug.h>
#include <time.h>

char* log_prepend(void) { return "YES: "; }

int main(void) {
  d_use_timestamp(1);
  d_set_timestamp_fmt("[%H:%M]");
  _l("Yes this is an output before!\n");

  d_set_format_func(log_prepend);

  _l("Hello yes!\n");
  return 0;
}
