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
#include <signal.h>


#include <sys/time.h>
#include <time.h>

#ifdef _WIN32
#include <io.h> /* _setmode() */
#include <fcntl.h> /* _O_BINARY */
#endif

#include "jtag.h"
#include "lattice_cmds.h"
#include "stdbool.h"

uint32_t d32[3] = { 0 };

uint32_t log_caps = 0;
uint8_t counter = 0;
bool capturing = false;

void shift_bit(FILE* event_file, bool b) {
  if(counter == 0) {
    if(b) counter = 96;
    else {
    fflush(event_file);
    }
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

      if((++log_caps % 10000) == 0) {
      static double lastTime = 0;
        fprintf(stderr, "%f %d %f\r\n", theTime, log_caps, 10000 / (theTime - lastTime));
        lastTime = theTime;
      }
      fwrite(d32, 1, sizeof(d32), event_file);
      //GlobalLogger_handle(ctx, &tx, 0);
      d32[0] = d32[1] = d32[2] = 0;
    }
  }
}

static volatile int keepRunning = 1;
void intHandler(int dummy) {
    if(keepRunning == 0) exit(-1);

    keepRunning = 0;
}

int main(int argc, char **argv)
{
  signal(SIGINT, intHandler);
  fprintf(stderr, "init..\n");
  FILE* event_file = argc > 1 ? fopen(argv[1], "wb") : stdout;

  int ifnum = 0;
  const char *devstr = NULL;
  int clkdiv = 1;

  jtag_init(ifnum, devstr, clkdiv);

  int numDev = jtag_query_device_count();

  uint32_t id[8] = {};
  jtag_query_devices(id, numDev);

  uint32_t ir_lengths[8] = {};
  int loggerIdx = -1;

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

  fprintf(stderr, "Shifting in IR: %lx\n", idata);
  jtag_go_to_state(STATE_SHIFT_IR);
  jtag_tap_shift((uint8_t*)&idata, (uint8_t*)&idata, cnt, true);

  jtag_go_to_state(STATE_SHIFT_DR);

  while(keepRunning) {
    uint8_t data_read[4096*2*2] = {};
    jtag_tap_shift(data_read, data_read, 8*sizeof(data_read), false);
    for(int i = 0;i < sizeof(data_read) * 8;i++) {
      bool b = (data_read[i / 8] >> (i % 8)) & 1;
      shift_bit(event_file, b);
    }
  }

  fclose(event_file);
  fprintf(stderr, "Exiting...\n");

  return 0;
}
