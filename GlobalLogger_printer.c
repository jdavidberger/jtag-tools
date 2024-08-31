#include "GlobalLogger.h"
#include <stddef.h>

#define NAME_FIELD(field) #field ": %llx "

#define VALUE_FIELD(field) , ((long long unsigned)pkt.field)

void GlobalLogger_handle_transaction(GlobalLogger_ctx* ctx, uint8_t id, const struct GlobalLogger_transaction* tx) {}

#define HANDLE_DEFINE(DEF) \
void GlobalLogger_handle_ ## DEF (GlobalLogger_ctx* ctx, uint64_t time, uint8_t id, const GlobalLogger_##DEF##_t pkt) { \
  printf("%-16s [%3d] (0x%08llx) " GlobalLogger_## DEF ##_FIELDS(NAME_FIELD) "\n", #DEF, id, (long long unsigned)time GlobalLogger_## DEF ##_FIELDS(VALUE_FIELD)); \
}
GlobalLogger_DEFINITIONS(HANDLE_DEFINE)