/**
 * @file um_tiledb.h
 *
**/

#ifndef UM_TILEDB_H
#define UM_TILEDB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiledb/tiledb.h>

extern int debug;

typedef struct um_tiledb_t {
    tiledb_ctx_t* ctx;
    tiledb_array_t* array;
// metadata 
    float *lon_list;
    float *lat_list;
    float *dep_list;
    size_t lon_cnt;
    size_t lat_cnt;
    size_t dep_cnt;
} um_tiledb_t;

// if latlon is set, starting condition is lat/lon/dep
// else              starting condition is indieces
typedef struct muscal_pt_info_t {
    int latlon;
    float lon;
    float lat;
    float dep;
    int lon_idx;
    int lat_idx;
    int dep_idx;
    float lon_fraction;
    float lat_fraction;
    float dep_fraction;
} muscal_pt_info_t;


typedef struct muscal_pt_property_t {
    float vp;
    float vs;
    float rho;
} muscal_pt_property_t;


void setup_tiledb(um_tiledb_t *tdb);
void free_tiledb(um_tiledb_t *tdb);

int open_tiledb(um_tiledb_t *tdb, const char  *tiledb_array_name);
int fill_metadata_tiledb(um_tiledb_t *tdb);
int fill_point_tiledb(um_tiledb_t *tdb, muscal_pt_info_t *pt, muscal_pt_property_t *data);

int fill_pt_info(um_tiledb_t *tdb, muscal_pt_info_t *pt);

void dump_coords_tiledb(um_tiledb_t *tdb, const char *fname, FILE *fp);
void dump_point_tiledb(muscal_pt_info_t *pt, muscal_pt_property_t *data, const char *fname, FILE *fp);

#endif
