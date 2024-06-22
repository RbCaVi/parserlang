#include "pv_constants.h"

const pv PV_NULL = {PVP_FLAGS_NULL, 0, 0, 0, {0}};
const pv PV_INVALID = {PVP_FLAGS_INVALID, 0, 0, 0, {0}};
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