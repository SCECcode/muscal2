/**
 * @file muscaltdb_dataset.h
 *
**/

#ifndef MUSCALTDB_DATASET_H
#define MUSCALTDB_DATASET_H

// setup for future possible expansion
#define MUSCALTDB_DATASET_MAX 1

#define MUSCALTDB_CACHE_LAYER_MAX 20
#define MUSCALTDB_CACHE_COL_MAX 10

#include "kdtree_util.h"
#include "muscaltdb.h"
#include "tiledb/tiledb.h"

typedef struct muscaltdb_properties_t muscaltdb_properties_t;

/** The MUSCALTDB, a dataset's working structure. */
/** 
       tiledb: tildb_ctx
          -file
              config ..nx/ny/nz

**/
typedef struct muscaltdb_dataset_t {
	/** Number of x(lon) points */
	int nx;
	/** Number of y(lat) points */
	int ny;
	/** Number of z(dep) points */
	int nz;

        /* surfaces  */
        KDNodeSetup *kdsurface;

	/** list of longitudes **/
	float *longitudes;
	/** list of latitudes **/
	float *latitudes;
	/** list of depths **/
	float *depths;

	int elems;

// for tiledb dataset
        tiledb_ctx_t *tiledb_ctx;
        tiledb_array_t *tiledb_array;
        char *tiledb_uri;

} muscaltdb_dataset_t;

typedef struct muscaltdb_pt_info_t {
	float lon;
	float lat;
	float dep;
        int lon_idx;
	int lat_idx;
	int dep_idx;
	float lon_percent;
	float lat_percent;
	float dep_percent;
} muscaltdb_pt_info_t;

/* utilitie functions */
muscaltdb_dataset_t * muscaltdb_read_dataset(char *datadir, char *datafile);
void muscaltdb_read_surface(muscaltdb_dataset_t *model, int count, char *datadir, char *surface_file);

int get_one_property(muscaltdb_dataset_t *model, muscaltdb_pt_info_t *pt, muscaltdb_properties_t *data);
int get_interp_property(muscaltdb_dataset_t *model, muscaltdb_pt_info_t *pt, muscaltdb_properties_t *data);
int get_1dnn_property(muscaltdb_dataset_t *model, muscaltdb_pt_info_t *pt, muscaltdb_properties_t *data);

void add_surface_data(muscaltdb_dataset_t  *model, char *sfile, int s_count);
void dump_dataset_metadata(muscaltdb_dataset_t *model);
int free_muscaltdb_dataset(muscaltdb_dataset_t *model);

#endif

