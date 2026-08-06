/*
 * um_tiledb.c
 * 
*/

#include "um_tiledb.h"

int debug=0;

void setup_tiledb(um_tiledb_t *tdb) {
    tdb->ctx = NULL;
    tdb->array = NULL;
    tdb->dep_cnt=0;
    tdb->lon_cnt=0;
    tdb->lat_cnt=0;
}

void free_tiledb(um_tiledb_t *tdb) {
    tiledb_array_close(tdb->ctx, tdb->array);
    tiledb_array_free(&tdb->array);
    tiledb_ctx_free(&tdb->ctx);
}

int open_tiledb(um_tiledb_t *tdb, const char  *tiledb_array_name) {
    // Allocate application context
    tiledb_ctx_alloc(NULL, &tdb->ctx);

    // Open the array in READ mode
    tiledb_array_alloc(tdb->ctx, tiledb_array_name, &tdb->array);
    if (tiledb_array_open(tdb->ctx, tdb->array, TILEDB_READ) != TILEDB_OK) {
        fprintf(stderr, "[ERROR] Unable to open TileDB array '%s' for reading.\n", tiledb_array_name);
        free_tiledb(tdb);
        return -1;
    }
    return 0;
}

void _check_list(float *flist, int idx) {
   fprintf(stderr,">>> %d: %.3f\n", idx, flist[idx]);
}

int fill_metadata_tiledb(um_tiledb_t *tdb) {
    tiledb_ctx_t* ctx = tdb->ctx;
    tiledb_array_t* array = tdb->array;

    tiledb_datatype_t val_type;
    uint32_t val_num;
    const void* val_ptr = NULL;

    // Fetch Depth Metadata Component
    tiledb_array_get_metadata(ctx, array, "coords_depth", &val_type, &val_num, &val_ptr);
    tdb->dep_list = (float *) malloc((val_num) * sizeof(float));
    if (!tdb->dep_list) {
        fprintf(stderr, "Out of memory allocating dep_list\n");
        return -1;
        } else {
          tdb->dep_cnt=(size_t)val_num;
          memcpy( tdb->dep_list, (float *) val_ptr,(tdb->dep_cnt * sizeof(float)));
    }

    // Fetch Latitude Metadata Component
    tiledb_array_get_metadata(ctx, array, "coords_latitude", &val_type, &val_num, &val_ptr);
    tdb->lat_list = (float *) malloc((val_num) * sizeof(float));
    if (!tdb->lat_list) {
        fprintf(stderr, "Out of memory allocating lat_list\n");
        return -1;
        } else {
          tdb->lat_cnt=(size_t) val_num;
          memcpy( tdb->lat_list, (float *) val_ptr,(tdb->lat_cnt * sizeof(float)));
    }

    // Fetch Longitude Metadata Component
    tiledb_array_get_metadata(ctx, array, "coords_longitude", &val_type, &val_num, &val_ptr);
    tdb->lon_list = (float *) malloc((val_num) * sizeof(float));
    if (!tdb->lon_list) {
        fprintf(stderr, "Out of memory allocating lon_list\n");
        return -1;
        } else {
          tdb->lon_cnt=(size_t) val_num;
          memcpy( tdb->lon_list, (float *) val_ptr, (tdb->lon_cnt * sizeof(float)));
    }
    return 0;
}


int _find_nearest_and_fraction(float* coord_array, uint32_t total_elements, 
                               float target_value, int *nearest_idx, float *fraction) {
    if (total_elements <= 1) {
        *nearest_idx = 0;
        *fraction = 0.0f;
        return 0;
    }
    
    int ascending = (coord_array[total_elements - 1] > coord_array[0]);
    float min_val = ascending ? coord_array[0] : coord_array[total_elements - 1];
    float max_val = ascending ? coord_array[total_elements - 1] : coord_array[0];

    if (target_value <= min_val) {
        *nearest_idx = ascending ? 0 : (int32_t)(total_elements - 1);
        *fraction = 0.0f;
        return 0;
    }
    if (target_value >= max_val) {
        *nearest_idx = ascending ? (int32_t)(total_elements - 1) : 0;
        *fraction = 0.0f;
        return 0;
    }

    for (uint32_t i = 0; i < total_elements - 1; i++) {
        float v0 = coord_array[i];
        float v1 = coord_array[i + 1];
        
        if ((target_value >= v0 && target_value <= v1) || (target_value <= v0 && target_value >= v1)) {
            float cell_width = v1 - v0;
            float raw_fraction = (target_value - v0) / cell_width;
            
            if (raw_fraction <= 0.5f) {
                *nearest_idx = (int32_t)i;
                *fraction = raw_fraction;
            } else {
                *nearest_idx = (int32_t)(i + 1);
                *fraction = raw_fraction - 1.0f; 
            }
            return 0;
        }
    }
    return 0;
}


int _find_cell_location(float* coord_array, uint32_t total_elements, 
                         uint32_t target_idx, float *val) {

    if (coord_array && target_idx < total_elements) {
        *val = coord_array[target_idx];
        } else {
        fprintf(stderr, "[WARN] Index out of bounds or metadata missing.\n");
    }
    return 0;
}
// with lon/lat/dep,  fill in idx/fraction
// or
// with idx, fill in lat/lon/dep
int fill_pt_info(um_tiledb_t *tdb, muscal_pt_info_t *pt) {
    if(pt->latlon) {
      _find_nearest_and_fraction(tdb->dep_list, tdb->dep_cnt, pt->dep, &pt->dep_idx, &pt->dep_fraction);
      _find_nearest_and_fraction(tdb->lon_list, tdb->lon_cnt, pt->lon, &pt->lon_idx, &pt->lon_fraction);
      _find_nearest_and_fraction(tdb->lat_list, tdb->lat_cnt, pt->lat, &pt->lat_idx, &pt->lat_fraction);
      } else {
        _find_cell_location(tdb->dep_list, tdb->dep_cnt, pt->dep_idx, &pt->dep);
        _find_cell_location(tdb->lon_list, tdb->lon_cnt, pt->lon_idx, &pt->lon);
        _find_cell_location(tdb->lat_list, tdb->lat_cnt, pt->lat_idx, &pt->lat);
    }
    return 0;
}

int fill_point_tiledb(um_tiledb_t *tdb, muscal_pt_info_t *pt, muscal_pt_property_t *data) {
    tiledb_ctx_t* ctx = tdb->ctx;
    tiledb_array_t* array = tdb->array;
    tiledb_query_t* query = NULL;
    tiledb_subarray_t* subarray = NULL; 
        
    float val_vp = 0.0f,
          val_vs = 0.0f,
          val_rho = 0.0f;

    uint64_t bytes_vp = sizeof(float),
          bytes_vs = sizeof(float),
          bytes_rho = sizeof(float);

    tiledb_query_alloc(ctx, array, TILEDB_READ, &query);
    tiledb_query_set_layout(ctx, query, TILEDB_ROW_MAJOR);
    tiledb_subarray_alloc(ctx, array, &subarray);

    int32_t depth_range[] = {pt->dep_idx, pt->dep_idx};
    int32_t lat_range[]   = {pt->lat_idx, pt->lat_idx};
    int32_t lon_range[]   = {pt->lon_idx, pt->lon_idx};

    tiledb_subarray_add_range(ctx, subarray, 0, &depth_range[0], &depth_range[1], NULL);
    tiledb_subarray_add_range(ctx, subarray, 1, &lat_range[0],  &lat_range[1],  NULL);
    tiledb_subarray_add_range(ctx, subarray, 2, &lon_range[0],  &lon_range[1],  NULL);
    tiledb_query_set_subarray_t(ctx, query, subarray);

    tiledb_query_set_data_buffer(ctx, query, "vp", &val_vp, &bytes_vp);
    tiledb_query_set_data_buffer(ctx, query, "vs", &val_vs, &bytes_vs);
    tiledb_query_set_data_buffer(ctx, query, "rho", &val_rho, &bytes_rho);

    if (tiledb_query_submit(ctx, query) != TILEDB_OK) {
        tiledb_error_t* err = NULL;
        tiledb_ctx_get_last_error(ctx, &err);
        const char* msg = NULL;
        tiledb_error_message(err, &msg);
        fprintf(stderr, "[ERROR] TileDB query read failed: %s\n", msg);
        tiledb_error_free(&err);
        } else {

          tiledb_query_status_t status;
          tiledb_query_get_status(ctx, query, &status);
          if (status == TILEDB_COMPLETED) {
              data->vp=val_vp;
              data->vs= val_vs;
              data->rho=val_rho;
            } else {
                fprintf(stderr, "[ERROR] Query finished with unexpected status flag: %d\n", status);
          }
    }

    if (subarray) tiledb_subarray_free(&subarray);
    if (query) tiledb_query_free(&query);

    return 0;
}


//============================================================================================

void dump_coords_tiledb(um_tiledb_t *tdb, const char *fname, FILE *fp) {
    int i;

    fprintf(fp,"===========================================\n");
    fprintf(fp,"%s\n", fname );
    fprintf(fp,"===========================================\n");
    fprintf(fp,"\nLongitude (%d)\n    ", tdb->lon_cnt);
    for(i=0; i< tdb->lon_cnt; i++) {
       fprintf(fp,"%.4f ",tdb->lon_list[i]);
       if((i+1) %8 == 0) fprintf(fp,"\n    ");
    }
    fprintf(fp,"\nLatitude (%d)\n    ", tdb->lat_cnt);
    for(i=0; i<tdb->lat_cnt; i++) {
       fprintf(fp,"%.4f ",tdb->lat_list[i]);
       if((i+1)%8 == 0) fprintf(fp,"\n    ");
    }
    fprintf(fp,"\nDepth (%d)\n    ", tdb->dep_cnt);
    for(i=0; i<tdb->dep_cnt; i++) {
       fprintf(fp,"%.4f ",tdb->dep_list[i]);
       if((i+1)%8 == 0) fprintf(fp,"\n    ");
    }
    fprintf(fp,"\n");
}


void dump_point_tiledb(muscal_pt_info_t *pt, muscal_pt_property_t *data, const char *fname, FILE *fp) {
    fprintf(fp,"===========================================\n");
    fprintf(fp,"%s\n", fname );
    fprintf(fp,"===========================================\n");
    fprintf(fp,"TARGET LOCATION SUMMARY\n");
    fprintf(fp,"===========================================\n");
    fprintf(fp,"Array Indices    -> Depth Idx: %5d | Lat Idx: %5d | Lon Idx: %5d\n", 
               pt->dep_idx, pt->lat_idx, pt->lon_idx);
    fprintf(fp,"Physical Coords  -> Depth: %.2f m | Latitude: %.4f° | Longitude: %.4f°\n", 
               pt->dep, pt->lat, pt->lon);
    if(pt->latlon) {
    fprintf(fp,"Cell Fraction    -> Depth Frac.: %.4f | Latitude Frac.: %.4f | Longitude Frac.: %.4f\n", 
               pt->dep_fraction, pt->lat_fraction, pt->lon_fraction);
    }
    fprintf(fp,"-------------------------------------------\n");
    fprintf(fp,"-------------------------------------------\n");
    fprintf(fp,"Material Values  -> VP  : %.4f m/s\n", data->vp);
    fprintf(fp,"                    VS  : %.4f m/s\n", data->vs);
    fprintf(fp,"                    RHO : %.4f kg/m^3\n", data->rho);
    fprintf(fp,"===========================================\n");
}
