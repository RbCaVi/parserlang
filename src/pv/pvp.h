void pvp_array_free(pv a);
void pvp_string_free(pv js);
void pvp_object_free(pv o);
void pvp_invalid_free(pv x);

int pvp_string_equal(pv a, pv b);
uint32_t pvp_string_hash(pv jstr);

void pvp_clamp_slice_params(int len, int *pstart, int *pend); // in pv_array.c but used by pv_string.c as well

#define KIND_MASK   0x1F
#define PFLAGS_MASK 0xE0

#define PVP_MAKE_FLAGS(kind, pflags) ((kind & KIND_MASK) | (pflags & PFLAGS_MASK))

typedef enum {
  PVP_PAYLOAD_NONE = 0,
  PVP_PAYLOAD_ALLOCATED = 0x80,
} payload_flags;

#define PVP_FLAGS(j)  ((j).kind_flags)
#define PVP_KIND(j)   (PVP_FLAGS(j) & KIND_MASK)

#define PVP_HAS_FLAGS(j, flags) (PVP_FLAGS(j) == flags)
#define PVP_HAS_KIND(j, kind)   (PVP_KIND(j) == kind)

typedef struct pv_refcnt {
  int count;
} pv_refcnt;

int pvp_refcnt_dec(pv_refcnt* c);
int pvp_refcnt_unshared(pv_refcnt* c);

static const pv_refcnt PV_REFCNT_INIT = {1};
