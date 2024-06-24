#include "pv_number.h"

#include "pvp.h"

#include <assert.h>
#include <math.h>
#include <float.h>

/*
 * Numbers
 */

pv pv_number(double x) {
  pv j = {
    PV_KIND_NUMBER,
    0, 0, 0, {.number = x}
  };
  return j;
}

double pv_number_value(pv j) {
  assert(PVP_HAS_KIND(j, PV_KIND_NUMBER));
  return j.u.number;
}

int pv_is_integer(pv j){
  if (!PVP_HAS_KIND(j, PV_KIND_NUMBER)){
    return 0;
  }

  double x = pv_number_value(j);

  double ipart;
  double fpart = modf(x, &ipart);

  return fabs(fpart) < DBL_EPSILON;
}

int pvp_number_is_nan(pv n) {
  assert(PVP_HAS_KIND(n, PV_KIND_NUMBER));
  return n.u.number != n.u.number;
}

int pvp_number_cmp(pv a, pv b) {
  assert(PVP_HAS_KIND(a, PV_KIND_NUMBER));
  assert(PVP_HAS_KIND(b, PV_KIND_NUMBER));

  double da = pv_number_value(a), db = pv_number_value(b);
  if (da < db) {
    return -1;
  } else if (da == db) {
    return 0;
  } else {
    return 1;
  }
}

static int pvp_number_equal(pv a, pv b) {
  return pvp_number_cmp(a, b) == 0;
}