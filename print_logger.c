#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
#include <io.h> /* _setmode() */
#include <fcntl.h> /* _O_BINARY */
#endif

#include "stdbool.h"

#include "GlobalLogger.h"

#include "sqlite3.h"

uint32_t d32[3] = { 0 };

uint32_t log_caps = 0;
uint8_t counter = 0;
bool capturing = false;
FILE* file = 0;

int main(int argc, char **argv)
{
  const char* raw_fn = argc > 1 ? argv[1] : "event_log.raw";

  GlobalLogger_ctx ctx = {};

  FILE* raw_file = fopen(raw_fn, "rb");
  struct GlobalLogger_transaction tx = { 0 };

  int txs = 0;

  while(fread(tx.l, sizeof(tx.l), 1, raw_file)) {
     txs++;
     GlobalLogger_handle(&ctx, &tx, 0);
  }

  return 0;
}
