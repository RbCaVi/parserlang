
order = [
  "main",
  "pl/util_pl",
  "pl/pl_stack",
  "pl/pl_opcodes",
  "pl/pl_dump",
  "pl/pl_ffi",
  "pv/pv",
  "pv/pvp",
  "pv/util",
  "pv/pv_alloc",
  "pv/pv_private",
  "pv/pv_array",
  "pv/pv_constants",
  "pv/pv_invalid",
  "pv/pv_number",
  "pv/pv_object",
  "pv/pv_string",
  "pv/pv_unicode",
  "pv/pv_utf8_tables",
  "pv/pv_aux",
]

def getstem(fname):
  fname,ext = fname.rsplit('.', 1)
  assert ext in ['c', 'h']
  return fname

data = []
with open(sys.argv[1]) as f:
  it = iter(f)
  for line in it:
    while line.endswith('\\\n'):
      line = line[-2:] + next(it)
    data.append(line)

rules = []

for line in data:
  isdefault = False
	target,prereqs = line.split(':')
  assert target.endswith('.o')
  stem = target[-2:]
  prereqs = set(prereqs)
  if stem + '.c' in prereqs and stem + '.h' in prereqs and 'pv/pv.h' in prereqs:
    prereqs.remove(stem + '.c')
    prereqs.remove(stem + '.h')
    prereqs.remove('pv/pv.h')
    isdefault = True
  prereqs = sorted(prereqs, lambda x: ([stem] + order).index(getstem(x)))
  print(f'{target}: {' '.join(prereqs)}')