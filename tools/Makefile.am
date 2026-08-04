# Autoconf/automake file

objects = um_tiledb.o

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS} ${CPPFLAGS} -I$(prefix)/include -I${UCVM_INSTALL_PATH}/lib/tiledb/include -I${UCVM_INSTALL_PATH}/lib/netcdf/include
AM_LDFLAGS = ${LDFLAGS} -L$(prefix)/lib ${LIBS} -L${UCVM_INSTALL_PATH}/lib/tiledb/lib64 -ltiledb -Wl,-rpath,${UCVM_INSTALL_PATH}/lib/tiledb/lib64 -L${UCVM_INSTALL_PATH}/lib/netcdf/lib -lnetcdf -lm


TARGETS = dump_cells dump_points dump_coords convert_nc_to_tiledb consolidate_tiledb


all: $(TARGETS) 

install:
clean:
	rm -rf $(TARGETS)
	rm -rf *.o

$(objects): %.o: %.c
	$(CC) -fPIC -DDYNAMIC_LIBRARY $(AM_CFLAGS) -o $@ -c $^

convert_nc_to_tiledb.o: convert_nc_to_tiledb.c
	$(CC)  -mavx2 $(AM_CFLAGS) -I${UCVM_INSTALL_PATH}/lib/netcdf/include -o $@ -c $^

convert_nc_to_tiledb : convert_nc_to_tiledb.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS) -L${UCVM_INSTALL_PATH}/lib/netcdf/lib -lnetcdf 

consolidate_tiledb.o: consolidate_tiledb.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^

consolidate_tiledb : consolidate_tiledb.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

dump_cells.o: dump_cells.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^

dump_cells : dump_cells.o um_tiledb.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

dump_coords.o: dump_coords.c
dump_points.o: dump_points.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^

dump_points : dump_points.o um_tiledb.o 
	$(CC) -o $@ $^ $(AM_LDFLAGS)

dump_coords.o: dump_coords.c
	$(CC) $(AM_CFLAGS) -o $@ -c $^

dump_coords : dump_coords.o um_tiledb.o
	$(CC) -o $@ $^ $(AM_LDFLAGS)


