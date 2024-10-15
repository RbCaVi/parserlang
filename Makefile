%.plc: %.pla pl_as.py src/pl/pl_opcodes_data.h src/pv/pv_number_ops_data.h
	python3 pl_as.py $< $@