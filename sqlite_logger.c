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

#include "jtag.h"
#include "lattice_cmds.h"
#include "stdbool.h"

#include "GlobalLogger.h"

#include "sqlite3.h"

uint32_t d32[3] = { 0 };


uint32_t log_caps = 0;
uint8_t counter = 0;
bool capturing = false;


void shift_bit(GlobalLogger_ctx* ctx, bool b) {
  if(counter == 0) {
    if(b) counter = 96;
  }

  if(counter != 0) 
  {
    uint8_t rev_c = 96 - counter--;
    d32[rev_c / 32] |= ((uint32_t)b) << (rev_c % 32);
    if(counter == 0) {
      capturing = 0;

      struct timeval tv = {0 };
      gettimeofday( &tv, NULL );      
      double theTime = tv.tv_sec + (0.000001 * tv.tv_usec);
      
      if((++log_caps % 1000) == 0) {
	fprintf(stderr, "%f %d\r\n", theTime, log_caps);
      }
      struct GlobalLogger_transaction tx = {
	.l = { d32[0], d32[1], d32[2] }
      };
      GlobalLogger_handle(ctx, &tx, 0);
      d32[0] = d32[1] = d32[2] = 0;
    }
  }
}

void GlobalLogger_init_sql(GlobalLogger_ctx* ctx, sqlite3* db); 


int main(int argc, char **argv)
{
  fprintf(stderr, "init..\n");
  const char* fn = argc > 1 ? argv[1] : "event_log.sqlite";
    
  int ifnum = 0;
  const char *devstr = NULL;
  int clkdiv = 1;
  
  jtag_init(ifnum, devstr, clkdiv);

  int numDev = jtag_query_device_count();
 	
  uint32_t id[8] = {};
  jtag_query_devices(id, numDev);
  
  uint32_t ir_lengths[8] = {};
  int loggerIdx = -1;
  
  jtag_go_to_state(STATE_SHIFT_DR);
  jtag_tap_shift((uint8_t*)id, (uint8_t*)id, 32 * numDev, true);

  for(int i = 0;i < numDev;i++) {
    fprintf(stderr, "Found devices %02x\r\n", id[i]);
    if(id[i] == 0x010003d1) {
      ir_lengths[i] = 8;
      loggerIdx = i;
    }
    else ir_lengths[i] = 5;
  }

  uint8_t cnt = 0;
  uint64_t idata = 0;
  for(int i = numDev - 1;i >= 0;i--) {
    if(i == loggerIdx) {
      idata = (idata << 8) | 0x24;
      cnt += 8;
    } else {
      idata = (idata << ir_lengths[i]) | ((1 << ir_lengths[i])-1);
      cnt += ir_lengths[i];
    }
  }
  
  printf("Shifting in IR: %lx\n", idata);
  jtag_go_to_state(STATE_SHIFT_IR);
  jtag_tap_shift((uint8_t*)&idata, (uint8_t*)&idata, cnt, true);
  
  jtag_go_to_state(STATE_SHIFT_DR);

  sqlite3* db = 0;
  sqlite3_open(fn, &db);
  
  GlobalLogger_ctx ctx = {};
  GlobalLogger_init_sql(&ctx, db);

  while(true) {
    uint8_t data_read[512] = {};
    jtag_tap_shift(data_read, data_read, 8*sizeof(data_read), false);
    for(int i = 0;i < sizeof(data_read) * 8;i++) {
      bool b = (data_read[i / 8] >> (i % 8)) & 1;
      shift_bit(&ctx, b);
    }
  }

  return 0;
}
