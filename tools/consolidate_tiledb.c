/**
   consolidate_tiledb.c
   
   consolidate array fragment in a tiledb model
    
   consolidate: Reads all 21 separate fragment directories, merges 
                their cell arrays and metadata into a single, 
                unified fragment folder, and writes a .vac marker file.
    vacuum: Reads the .vac marker file created during consolidation and
                deletes the 21 old fragment directories from disk.

Key Rule: TileDB arrays are append-only. Consolidation never modifies the 
original files; it generates a new, optimized file.

**/
#include <stdio.h>
#include <tiledb/tiledb.h>

void consolidate_and_vacuum(const char* array_uri) {
    tiledb_ctx_t* ctx = NULL;
    tiledb_ctx_alloc(NULL, &ctx);

    // 1. Consolidate fragments (merges the 21 fragment directories into 1)
    printf("Consolidating array at: %s...\n", array_uri);
    if (tiledb_array_consolidate(ctx, array_uri, NULL) == TILEDB_OK) {
        printf("Consolidation successful.\n");
    } else {
        fprintf(stderr, "Consolidation failed.\n");
    }

    // 2. Vacuum old fragments (deletes the old, unmerged fragment folders)
    printf("Vacuuming old fragments...\n");
    if (tiledb_array_vacuum(ctx, array_uri, NULL) == TILEDB_OK) {
        printf("Vacuuming successful.\n");
    } else {
        fprintf(stderr, "Vacuuming failed.\n");
    }

    tiledb_ctx_free(&ctx);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <path_to_tiledb_array>\n", argv[0]);
        return 1;
    }
    consolidate_and_vacuum(argv[1]);
    return 0;
}
