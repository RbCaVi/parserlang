/*
 * Invalid objects, with optional error messages
 */

#define PVP_FLAGS_INVALID_MSG   PVP_MAKE_FLAGS(PV_KIND_INVALID, PVP_PAYLOAD_ALLOCATED)

typedef struct {
  pv_refcnt refcnt;
  pv errmsg;
} pvp_invalid;

pv pv_invalid_with_msg(pv err) {
  pvp_invalid* i = pv_mem_alloc(sizeof(pvp_invalid));
  i->refcnt = PV_REFCNT_INIT;
  i->errmsg = err;

  pv x = {PVP_FLAGS_INVALID_MSG, 0, 0, 0, {&i->refcnt}};
  return x;
}

pv pv_invalid() {
  return PV_INVALID;
}

pv pv_invalid_get_msg(pv inv) {
  assert(PVP_HAS_KIND(inv, PV_KIND_INVALID));

  pv x;
  if (PVP_HAS_FLAGS(inv, PVP_FLAGS_INVALID_MSG)) {
    x = pv_ref(((pvp_invalid*)inv.u.ptr)->errmsg);
  }
  else {
    x = pv_null();
  }

  pv_unref(inv);
  return x;
}

int pv_invalid_has_msg(pv inv) {
  assert(PVP_HAS_KIND(inv, PV_KIND_INVALID));
  int r = PVP_HAS_FLAGS(inv, PVP_FLAGS_INVALID_MSG);
  pv_unref(inv);
  return r;
}

static void pvp_invalid_free(pv x) {
  assert(PVP_HAS_KIND(x, PV_KIND_INVALID));
  if (PVP_HAS_FLAGS(x, PVP_FLAGS_INVALID_MSG) && pvp_refcnt_dec(x.u.ptr)) {
    pv_unref(((pvp_invalid*)x.u.ptr)->errmsg);
    pv_mem_free(x.u.ptr);
  }
}