/**         
    dump_coords.c
    
    Usage: ./dump_coords
                
LOOK:           
    const char* tiledb_array_name = "model_MUSCAL_CANVAS_dll0.01_vardz_float32_cmpd.tiledb";

**/     

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiledb/tiledb.h>

#include "um_tiledb.h"

int main() {
    const char* tiledb_array_name = "model_MUSCAL_CANVAS_dll0.01_vardz_float32_cmpd.tiledb";
    int rc;

    um_tiledb_t tdb;

    setup_tiledb(&tdb);

    rc=open_tiledb(&tdb, tiledb_array_name);

    rc=fill_metadata_tiledb(&tdb);

    dump_coords_tiledb(&tdb, tiledb_array_name, stderr);

    free_tiledb(&tdb);

    return 0;
}

