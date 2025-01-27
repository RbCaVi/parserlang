prgm="$1"
rm -f ../${prgm}.plp &&
make -C .. ${prgm}.plp &&
make clean all &&
LD_LIBRARY_PATH=. ./plc_dumptree ../${prgm}.plp &&
LD_LIBRARY_PATH=. valgrind --leak-check=full ./plc_compile ../${prgm}.plp ${prgm}.plc &&
LD_LIBRARY_PATH=. valgrind --leak-check=full ./pl_run ${prgm}.plc