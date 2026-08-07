# Autoconf/automake file

# General compiler/linker flags
AM_CFLAGS = ${CFLAGS}
AM_FCFLAGS = ${FCFLAGS}
AM_LDFLAGS = ${LDFLAGS}

all:
if WITH_MUSCALTDB_LARGEDATA_DIR
	echo "USING WITH_MUSCALTDB_LARGEDATA_DIR in muscaltdb"
else
	echo "NOT USING WITH_MUSCALTDB_LARGEDATA_DIR in muscaltdb"
	if [ ! -d "muscaltdb" ] && [ ! -L "muscaltdb" ]; then \
		./make_data_files.py; \
	fi 
endif

install:
if WITH_MUSCALTDB_LARGEDATA_DIR
	if [ ! -d "${prefix}/data/muscaltdb" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		$(LN_S) -f $(MUSCALTDB_LARGEDATA_DIR) ${prefix}/data/muscaltdb; \
		echo "USING WITH_MUSCALTDB_LARGEDATA_DIR in muscaltdb"; \
	fi
	cp config ${prefix}/data
else
	if [ ! -d "${prefix}/data/muscaltdb" ]; then \
		$(MKDIR_P) ${prefix}/data; \
		mv muscaltdb ${prefix}/data; \
		$(LN_S) -f ${prefix}/data/muscaltdb .; \
		echo "NOT USING WITH_MUSCALTDB_LARGEDATA_DIR in muscaltdb"; \
	fi
	cp config ${prefix}/data
endif

clean:
	echo "clean"
