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
  enum pv_iter_val_type {
  	ARRAY,
  	OBJECT,
  } val_type;
  enum pv_iter_type {
  	KEYS,
  	VALUES,
  	ENTRIES,
  } type;
  pv val;
  union {
	  uint32_t aiter;
	  int oiter;
	};
} plp_iter_data;

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
		i->val_type = ARRAY;
		i->aiter = 0;
	} else {
		i->val_type = OBJECT;
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
	case ARRAY:
		if (i->aiter >= pv_array_length(pv_copy(i->val))) {
			out = pv_invalid();
			break;
		}
		switch (i->type) {
		case KEYS:
			out = pv_int((int)i->aiter);
			break;
		case VALUES:
			out = pv_array_get(pv_copy(i->val), i->aiter);
			break;
		case ENTRIES:
			out = PV_ARRAY(pv_int((int)i->aiter), pv_array_get(pv_copy(i->val), i->aiter));
			break;
		default:
			out = pv_invalid(); // just in case
			break;
		}
		break;
	case OBJECT:
		if (!pv_object_iter_valid(pv_copy(i->val), i->oiter)) {
			out = pv_invalid();
			break;
		}
		switch (i->type) {
		case KEYS:
			out = pv_object_iter_key(pv_copy(i->val), i->oiter);
			break;
		case VALUES:
			out = pv_object_iter_value(pv_copy(i->val), i->oiter);
			break;
		case ENTRIES:
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
	case ARRAY:
		i->aiter++;
		break;
	case OBJECT:
		if (!pv_object_iter_valid(pv_copy(i->val), i->oiter)) {
			break;
		}
		i->oiter = pv_object_iter_next(pv_copy(i->val), i->oiter);
		break;
	}
	return (pv){iter_kind, PV_FLAG_ALLOCATED, &(i->refcnt)};
}