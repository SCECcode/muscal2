/**         
    dump_points.c
    
    Usage: ./dump_points query_point
                
LOOK:           
    const char* tiledb_array_name = "model_MUSCAL_CANVAS_dll0.01_vardz_float32_cmpd.tiledb";
i:
-118.241 34.0025 45.2

**/     

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tiledb/tiledb.h>

#include "um_tiledb.h"

void usage() {
  printf("Usage: ./dump_points query_list\n\n");
  exit(0);
}

int main(int argc, char **argv) {

    char datafile[500];
    char line[200];
    FILE *fp;
    float lon,lat,depth;

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
      if (sscanf(line,"%f %f %f", &lon, &lat, &depth) == 3) {
          pt.lon=lon;
          pt.lat=lat; 
          pt.dep=depth;
          pt.latlon=1;
          fill_pt_info(&tdb, &pt);
          fill_point_tiledb(&tdb, &pt, &data);
          dump_point_tiledb(&pt, &data, tiledb_array_name,stderr);
          numread++;
      }
    }
    fclose(fp);
    free_tiledb(&tdb);
    return 0;
}

