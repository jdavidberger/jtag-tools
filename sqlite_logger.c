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

#include "lattice_cmds.h"
#include "stdbool.h"

#include "GlobalLogger.h"

#include "sqlite3.h"

uint32_t d32[3] = { 0 };

uint32_t log_caps = 0;
uint8_t counter = 0;
bool capturing = false;
FILE* file = 0;

void GlobalLogger_init_sql(GlobalLogger_ctx* ctx, sqlite3* db); 

static double get_time() {
      struct timeval tv = {0 };
      gettimeofday( &tv, NULL );
      return tv.tv_sec + (0.000001 * tv.tv_usec);
}

static volatile int keepRunning = 1;

int main(int argc, char **argv)
{


  const char* sql_fn = argc > 1 ? argv[1] : "event_log.sqlite";
  const char* raw_fn = argc > 2 ? argv[2] : 0;

  sqlite3* db = 0;
  sqlite3_open(sql_fn, &db);
  
  GlobalLogger_ctx ctx = {};
  GlobalLogger_init_sql(&ctx, db);
  FILE* raw_file = raw_fn ? fopen(raw_fn, "rb") : stdin;

  struct GlobalLogger_transaction tx = { 0 };

  double startTime = get_time();
  int txs = 0;

  while(fread(tx.l, sizeof(tx.l), 1, raw_file)) {
     txs++;
     GlobalLogger_handle(&ctx, &tx, 0);
  }

  fprintf(stderr, "%d %f\r\n", txs, txs / (get_time() - startTime));
  sqlite3_close(db);
  fprintf(stderr, "Exiting SQLite ...\n");

  return 0;
}
