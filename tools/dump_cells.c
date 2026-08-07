/**         
    dump_cells.c
    
    Usage: ./dump_cells query_cell
                
LOOK:           
    const char* tiledb_array_name = "model_MUSCAL_CANVAS_dll0.01_vardz_float32_cmpd.tiledb";
ii:
906 613 2

**/     

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiledb/tiledb.h>

#include "um_tiledb.h"

void usage() {
  printf("Usage: ./dump_cells query_list\n\n");
  exit(0);
}

int main(int argc, char **argv) {

    char datafile[500];
    char line[200];
    FILE *fp;
    int lon_idx,lat_idx,depth_idx;

    const char* tiledb_array_name = "model_MUSCAL_CANVAS_dll0.01_vardz_float32_cmpd.tiledb";
    int rc;
    um_tiledb_t tdb;
    muscaltdb_pt_info_t pt;
    muscaltdb_pt_property_t data;

    if(argc != 2) { usage(); }
    strcpy(datafile, argv[1]);

    setup_tiledb(&tdb);
    rc=open_tiledb(&tdb, tiledb_array_name);
    rc=fill_metadata_tiledb(&tdb);

    fp=fopen(datafile,"r");
    int numread=0;
// read all the points
    while (fgets(line, 200, fp) != NULL ) {
      if(line[0]=='#') continue;  // a comment line
      if (sscanf(line,"%d %d %d", &lon_idx, &lat_idx, &depth_idx) == 3) {
          pt.lon_idx=lon_idx;
          pt.lat_idx=lat_idx; 
          pt.dep_idx=depth_idx;
          pt.latlon=0;
          fill_pt_info(&tdb, &pt);
          fill_point_tiledb(&tdb, &pt, &data);
          dump_point_tiledb(&pt, &data, tiledb_array_name, stderr);
          numread++;
      }
    }
    fclose(fp);
    free_tiledb(&tdb);
    return 0;
}

