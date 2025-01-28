prgm="$1"
rm -f ../${prgm}.plp &&
make -C .. ${prgm}.plp &&
LD_LIBRARY_PATH=. ./plc_dumptree.exe ../${prgm}.plp &&
LD_LIBRARY_PATH=. ./plc_compile.exe ../${prgm}.plp ${prgm}.plc &&
LD_LIBRARY_PATH=. ./pl_run.exe ${prgm}.plc
