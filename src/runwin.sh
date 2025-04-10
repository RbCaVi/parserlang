prgm="$1"
rm -f ../${prgm}.plp &&
make -C .. ${prgm}.plp &&
make pv.so pl.so plc_dumptree plc_compile pl_run &&
LD_LIBRARY_PATH=. ./plc_dumptree.exe ../${prgm}.plp &&
LD_LIBRARY_PATH=. ./plc_compile.exe ../${prgm}.plp ${prgm}.plc &&
LD_LIBRARY_PATH=. ./pl_run.exe ${prgm}.plc
