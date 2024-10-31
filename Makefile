.PHONY: clean

%.plc: %.pla pl_as.py src/pl/pl_opcodes_data.h src/pv/pv_number_ops_data.h
	python3 pl_as.py $< $@

%.plp: %.pls pl_parse.py src/plc/plc_op_ids.h src/pv/pv_number_ops_data.h
	python3 pl_parse.py $< $@

clean:
	rm */*.plc
	rm */*.plp