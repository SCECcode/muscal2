# Autoconf/automake file

objects = muscaltdb_util.o muscaltdb_dataset.o cJSON.o kdtree_util.o

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} ${CPPFLAGS} -I$(prefix)/include -I${UCVM_INSTALL_PATH}/lib/tiledb/include
AM_LDFLAGS = ${LDFLAGS} -L$(prefix)/lib ${LIBS} -L${UCVM_INSTALL_PATH}/lib/tiledb/lib64 -ltiledb -Wl,-rpath,${UCVM_INSTALL_PATH}/lib/tiledb/lib64 -lm

TARGETS = muscaltdb_query libmuscaltdb.a libmuscaltdb.so 

all: $(TARGETS)

install:
	mkdir -p ${prefix}
	mkdir -p ${prefix}/lib
	mkdir -p ${prefix}/include
	mkdir -p ${prefix}/bin
	cp libmuscaltdb.so ${prefix}/lib
	cp libmuscaltdb.a ${prefix}/lib
	cp muscaltdb.h ${prefix}/include
	cp muscaltdb_query ${prefix}/bin

clean:
	rm -rf $(TARGETS)
	rm -rf *.o

libmuscaltdb.a: muscaltdb_static.o muscaltdb_util.o muscaltdb_dataset.o cJSON.o kdtree_util.o
	$(AR) rcs $@ $^

libmuscaltdb.so: muscaltdb.o muscaltdb_util.o muscaltdb_dataset.o cJSON.o kdtree_util.o
	$(CC) -shared $(AM_FCFLAGS) -o libmuscaltdb.so $^ $(AM_LDFLAGS)

muscaltdb.o: muscaltdb.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY -o $@ -c $^ $(AM_CFLAGS)

muscaltdb_static.o: muscaltdb.c
	$(CC) -o $@ -c $^ $(AM_CFLAGS)

$(objects): %.o: %.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY $(AM_CFLAGS) -o $@ -c $^

muscaltdb_query.o: muscaltdb_query.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^

muscaltdb_query : muscaltdb_query.o libmuscaltdb.a
	$(CC) -o $@ $^ $(AM_LDFLAGS)

