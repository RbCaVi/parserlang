#include "pv_constants.h"

#include "pvp.h"

#define PVP_FLAGS_NULL      PVP_MAKE_FLAGS(PV_KIND_NULL, PVP_PAYLOAD_NONE)
#define PVP_FLAGS_FALSE     PVP_MAKE_FLAGS(PV_KIND_FALSE, PVP_PAYLOAD_NONE)
#define PVP_FLAGS_TRUE      PVP_MAKE_FLAGS(PV_KIND_TRUE, PVP_PAYLOAD_NONE)

const pv PV_NULL = {PVP_FLAGS_NULL, 0, 0, 0, {0}};
const pv PV_FALSE = {PVP_FLAGS_FALSE, 0, 0, 0, {0}};
const pv PV_TRUE = {PVP_FLAGS_TRUE, 0, 0, 0, {0}};

pv pv_true() {
  return PV_TRUE;
}

pv pv_false() {
  return PV_FALSE;
}

pv pv_null() {
  return PV_NULL;
}

pv pv_bool(int x) {
  return x ? PV_TRUE : PV_FALSE;
}