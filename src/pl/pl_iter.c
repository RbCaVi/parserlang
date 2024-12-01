#include "pl_iter.h"

#include "pv_number.h"
#include "pv_array.h"
#include "pv_object.h"
#include "pv_private.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>

pv_kind iter_kind;

typedef struct {
  pv_refcnt refcnt;
  enum pv_iter_out_type {
  	ARRAYK,
  	OBJECTK,
  	ARRAYV,
  	OBJECTV,
  	ARRAYKV,
  	OBJECTKV,
  } iter_type;
  pv val;
  union {
	  uint32_t aiter;
	  int oiter;
	};
} plp_iter_data;

typedef enum {
  	KEYS,
  	VALUES,
  	ENTRIES,
} pv_iter_type;

static plp_iter_data *plp_iter_get_data(pv val) {
	plp_iter_data *i = (plp_iter_data*)val.data;
	return i;
}

static void pv_iter_free(pv val) {
	plp_iter_data *i = plp_iter_get_data(val);

	pv_free(i->val);
	free(i);
}

void pl_iter_install() {
	pv_register_kind(&iter_kind, "iter", pv_iter_free);
}

static plp_iter_data *plp_iter_alloc() {
	plp_iter_data *i = pv_alloc(sizeof(plp_iter_data));
	i->refcnt = PV_REFCNT_INIT;
	return i;
}

static pv plp_setup_iter(pv val, enum pv_iter_type type) {
	plp_iter_data *i = plp_iter_alloc();
	i->val = val;
	i->type = type;
	if (pv_get_kind(val) == array_kind) {
		switch (type) {
		case KEYS:
			i->val_type = ARRAYK;
		case VALUES:
			i->val_type = ARRAYV;
		case ENTRIES:
			i->val_type = ARRAYKV;
		default:
			abort();
		}
		i->aiter = 0;
	} else {
		switch (type) {
		case KEYS:
			i->val_type = OBJECTK;
		case VALUES:
			i->val_type = OBJECTV;
		case ENTRIES:
			i->val_type = OBJECTKV;
		default:
			abort();
		}
		i->oiter = pv_object_iter(pv_copy(val));
	}
	return (pv){iter_kind, PV_FLAG_ALLOCATED, &(i->refcnt)};
}

pv pl_iter(pv val) {
	if (pv_get_kind(val) == array_kind) {
		return plp_setup_iter(val, VALUES);
	} else {
		return plp_setup_iter(val, KEYS);
	}
}

pv pl_iter_keys(pv val) {
	return plp_setup_iter(val, KEYS);
}

pv pl_iter_values(pv val) {
	return plp_setup_iter(val, VALUES);
}

pv pl_iter_entries(pv val) {
	return plp_setup_iter(val, ENTRIES);
}

pv pl_iter_value(pv val) {
	assert(val.kind == iter_kind);
	plp_iter_data *i = plp_iter_get_data(val);
	pv out;
	switch (i->val_type) {
	case ARRAYK:
	case ARRAYV:
	case ARRAYKV:
		if (i->aiter >= pv_array_length(pv_copy(i->val))) {
			out = pv_invalid();
			break;
		}
		switch (i->val_type) {
		case ARRAYK:
			out = pv_int((int)i->aiter);
			break;
		case ARRAYV:
			out = pv_array_get(pv_copy(i->val), (int)i->aiter);
			break;
		case ARRAYKV:
			out = PV_ARRAY(pv_int((int)i->aiter), pv_array_get(pv_copy(i->val), (int)i->aiter));
			break;
		default:
			out = pv_invalid(); // just in case
			break;
		}
		break;
	case OBJECTK:
	case OBJECTV:
	case OBJECTKV:
		if (!pv_object_iter_valid(pv_copy(i->val), i->oiter)) {
			out = pv_invalid();
			break;
		}
		switch (i->val_type) {
		case OBJECTK:
			out = pv_object_iter_key(pv_copy(i->val), i->oiter);
			break;
		case OBJECTV:
			out = pv_object_iter_value(pv_copy(i->val), i->oiter);
			break;
		case OBJECTKV:
			out = PV_ARRAY(pv_object_iter_key(pv_copy(i->val), i->oiter), pv_object_iter_value(pv_copy(i->val), i->oiter));
			break;
		default:
			out = pv_invalid(); // just in case
			break;
		}
		break;
	default:
		out = pv_invalid();
		break;
	}
	pv_free(val);
	return out;
}

static plp_iter_data *plp_iter_realloc(plp_iter_data *iin) {
	if (pvp_refcnt_unshared(&(iin->refcnt))) {
		return iin; // literally nothing to be done
	}
	plp_iter_data *i = pv_alloc(sizeof(plp_iter_data));
	memcpy(i, iin, sizeof(plp_iter_data));
	i->refcnt = PV_REFCNT_INIT;
	pv_copy(i->val);
	pvp_decref(&(iin->refcnt));
	return i;
}

pv pl_iter_next(pv val) {
	assert(val.kind == iter_kind);
	plp_iter_data *i = plp_iter_realloc(plp_iter_get_data(val));
	switch (i->val_type) {
	case ARRAYK:
	case ARRAYV:
	case ARRAYKV:
		i->aiter++;
		break;
	case OBJECTK:
	case OBJECTV:
	case OBJECTKV:
		if (!pv_object_iter_valid(pv_copy(i->val), i->oiter)) {
			break;
		}
		i->oiter = pv_object_iter_next(pv_copy(i->val), i->oiter);
		break;
	}
	return (pv){iter_kind, PV_FLAG_ALLOCATED, &(i->refcnt)};
}
