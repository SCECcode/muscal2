/**
 * @file muscal.c
 * @brief Main file for muscal model
 * @version 1.0
 *
 * @section DESCRIPTION
 *
 */

#include <limits.h>
#include "ucvm_model_dtypes.h"
#include "muscal.h"
#include "cJSON.h"

int muscal_ucvm_debug=0;
int muscal_ucvm_debug_stats=0;
int muscal_ucvm_debug_detail=0;
FILE *stderrfp=NULL;

char *tiledb_stats_json = NULL;

/** The config of the model */
char *muscal_config_string=NULL;
int muscal_config_sz=0;

// Constants
/** The version of the model. */
const char *muscal_version_string = "muscal";

// Variables
/** Set to 1 when the model is ready for query. */
int muscal_is_initialized = 0;

char muscal_data_directory[128];

/** Configuration parameters. */
muscal_configuration_t *muscal_configuration;
/** Holds all info extracted from tiledb data */
muscal_dataset_t *muscal_dataset;

/**
 * Initializes the muscal plugin model within the UCVM framework. In order to initialize
 * the model, we must provide the UCVM install path and optionally a place in memory
 * where the model already exists.
 *
 * @param dir The directory in which UCVM has been installed.
 * @param label A unique identifier for the velocity model.
 * @return Success or failure, if initialization was successful.
 */
int muscal_init(const char *dir, const char *label) {
    int tempVal = 0;
    char configbuf[512];
    double north_height_m = 0, east_width_m = 0, rotation_angle = 0;

    if(muscal_ucvm_debug || muscal_ucvm_debug_stats){
      stderrfp = fopen("muscal_debug.log", "w+");
      fprintf(stderrfp,"\n===== START muscal ===== \n\n");
    }

    if(muscal_ucvm_debug_stats){ tiledb_stats_enable(); }

    // Initialize variables.
    
    muscal_configuration = calloc(1, sizeof(muscal_configuration_t));
    muscal_config_string = calloc(MUSCAL_CONFIG_MAX, sizeof(char));
    muscal_config_string[0]='\0';
    muscal_config_sz=0;

    // Configuration file location.
    sprintf(configbuf, "%s/model/%s/data/config", dir, label);

    // Read the configuration file.
    if (muscal_read_configuration(configbuf, muscal_configuration) != UCVM_MODEL_CODE_SUCCESS) {
           // Try another, when is running in standalone mode..
       sprintf(configbuf, "%s/data/config", dir);
       if (muscal_read_configuration(configbuf, muscal_configuration) != UCVM_MODEL_CODE_SUCCESS) {
           muscal_print_error("No configuration file was found to read from.");
           return UCVM_MODEL_CODE_ERROR;
           } else {
           // Set up the data directory.
               sprintf(muscal_data_directory, "%s/data/%s", dir, muscal_configuration->model_dir);
       }
       } else {
           // Set up the data directory.
           sprintf(muscal_data_directory, "%s/model/%s/data/%s", dir, label, muscal_configuration->model_dir);
    }

    muscal_dataset = muscal_read_dataset(muscal_data_directory, muscal_configuration->dataset_file);
    if (muscal_dataset == NULL) {
        muscal_print_error("No model file was found to read from.");
        return FAIL;
    }
    if( muscal_configuration->enable_1d) {
        muscal_read_surface(muscal_dataset, muscal_configuration->surface_count,
                muscal_data_directory, muscal_configuration->surface_file);
    }

    // setup config_string 
    sprintf(muscal_config_string,"config = %s\n",configbuf);
    muscal_config_sz=1;

    // Let everyone know that we are initialized and ready for business.
    muscal_is_initialized = 1;

    return SUCCESS;
}

/**
 * Queries muscal at the given points and returns the data that it finds.
 *
 * @param points The points at which the queries will be made.
 * @param data The data that will be returned (Vp, Vs, rho, Qs, and/or Qp).
 * @param numpoints The total number of points to query.
 * @return SUCCESS or FAIL.
 */
int muscal_query(muscal_point_t *points, muscal_properties_t *data, int numpoints) {

if(muscal_ucvm_debug){ fprintf(stderrfp,"\ncalling muscal_query with %d numpoints\n",numpoints); }

    muscal_dataset_t *model=muscal_dataset;

    float *lon_list=model->longitudes;
    float *lat_list=model->latitudes;
    float *dep_list=model->depths;
    int nx=model->nx;
    int ny=model->ny;
    int nz=model->nz;

    int lon_idx;
    int lat_idx;
    int dep_idx;

    int offset;

//  hold coord point's info
    muscal_pt_info_t *pt_info = (muscal_pt_info_t  *) malloc(numpoints * sizeof(muscal_pt_info_t));
    if (!pt_info) { fprintf(stderr, "pt_info: malloc failed\n");}

// iterate through all the points and compose the buffers for all index
    for(int i=0; i<numpoints; i++) {
        data[i].vp = -1;
        data[i].vs = -1;
        data[i].rho = -1;
        data[i].qp = -1;
        data[i].qs = -1;

        pt_info[i].lon=points[i].longitude;
        pt_info[i].lat=points[i].latitude;
        pt_info[i].dep=points[i].depth;
        if(muscal_ucvm_debug_stats) {
            fprintf(stderrfp,"POINT: %lf %lf %lf\n", points[i].longitude,points[i].latitude,points[i].depth);
        }

        pt_info[i].lon_idx=find_buffer_idx_clamped(lon_list,nx,pt_info[i].lon);
        pt_info[i].lat_idx=find_buffer_idx_clamped(lat_list,ny,pt_info[i].lat);
        pt_info[i].dep_idx=find_buffer_idx_clamped(dep_list,nz,pt_info[i].dep);

     /* check if out of range */
        if(pt_info[i].lon_idx < 0 || pt_info[i].lat_idx < 0 || pt_info[i].dep_idx < 0) {
          continue;
        }

        if(muscal_configuration->interpolation) { // fill cell percent
            pt_info[i].lon_percent=find_cell_percent(lon_list,pt_info[i].lon,pt_info[i].lon_idx);
            pt_info[i].lat_percent=find_cell_percent(lat_list,pt_info[i].lat,pt_info[i].lat_idx);
            pt_info[i].dep_percent=find_cell_percent(dep_list,pt_info[i].dep,pt_info[i].dep_idx);
        }
    }

    // should be in the in-memory 
    for(int i=0; i<numpoints; i++) {
        if(!muscal_configuration->interpolation) { 
        // no interp
            get_one_property(model, &(pt_info[i]), &(data[i]));
            } else {
                get_interp_property(model, &(pt_info[i]), &(data[i]));
        }

// If result is None, then need to process
// with 1d nearest neighbor surface property
        if(isnan(data[i].vp) && isnan(data[i].vs) && muscal_configuration->enable_1d) {
            get_1dnn_property(model, &(pt_info[i]), &(data[i]));
        }
    } 

    if(muscal_ucvm_debug_stats){
        fprintf(stderrfp,"--TileDB Query Performance Stats --\n");
        //tiledb_stats_dump(stderrfp);
        //tiledb_stats_raw_dump_str(&tiledb_stats_json);
        if(tiledb_stats_json != NULL) {
           fprintf(stderrfp,"JSON stats: \n   %s\n", tiledb_stats_json);
           tiledb_stats_free_str(&tiledb_stats_json);
        }
    }   

    return SUCCESS;
}

/**
 */
void muscal_setdebug() {
   muscal_ucvm_debug=1;
}               

/**
 * Called when the model is being discarded. Free all variables.
 *
 * @return SUCCESS
 */
int muscal_finalize() {

    muscal_is_initialized = 0;

    if (muscal_configuration) {
        muscal_configuration_finalize(muscal_configuration);
    }
    if (muscal_dataset) {
        free_muscal_dataset(muscal_dataset);
    }

    if(muscal_ucvm_debug || muscal_ucvm_debug_stats){
     fprintf(stderrfp,"::DONE::\n"); 
     fclose(stderrfp);
    }

    if(muscal_ucvm_debug_stats){ tiledb_stats_disable(); }   

    return SUCCESS;
}

/**
 * Returns the version information.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int muscal_version(char *ver, int len)
{
  int verlen;
  verlen = strlen(muscal_version_string);
  if (verlen > len - 1) {
    verlen = len - 1;
  }
  memset(ver, 0, len);
  strncpy(ver, muscal_version_string, verlen);
  return 0;
}

/**
 * Returns the model config information.
 *
 * @param key Config key string to return.
 * @param sz number of config terms.
 * @return Zero
 */
int muscal_config(char **config, int *sz)
{
  int len=strlen(muscal_config_string);
  if(len > 0) {
    *config=muscal_config_string;
    *sz=muscal_config_sz;
    return SUCCESS;
  }
  return FAIL;
}


/**
 * Reads the muscal_configuration file describing the various properties of MUSCAL and populates
 * the muscal_configuration struct. This assumes muscal_configuration has been "calloc'ed" and validates
 * that each value is not zero at the end.
 *
 * @param file The muscal_configuration file location on disk to read.
 * @param config The muscal_configuration struct to which the data should be written.
 * @return Number of dataset expected in the model
 */
int muscal_read_configuration(char *file, muscal_configuration_t *config) {
    FILE *fp = fopen(file, "r");
    char key[40];
    char value[1000];
    char line_holder[2000];

    // If our file pointer is null, an error has occurred. Return fail.
    if (fp == NULL) { return UCVM_MODEL_CODE_ERROR; }

    // Read the lines in the muscal_configuration file.
    while (fgets(line_holder, sizeof(line_holder), fp) != NULL) {

        if (line_holder[0] != '#' && line_holder[0] != ' ' && line_holder[0] != '\n') {
            _splitline(line_holder, key, value);
            if(muscal_ucvm_debug){ fprintf(stderrfp, "a config line: %s\n", line_holder); }

            // Which variable are we editing?
            if (strcmp(key, "utm_zone") == 0) config->utm_zone = atoi(value);
            if (strcmp(key, "model_dir") == 0) sprintf(config->model_dir, "%s", value);
            if (strcmp(key, "interpolation") == 0) { 
                config->interpolation=0;
                if (strcmp(value,"on") == 0) { config->interpolation=1;
if(muscal_ucvm_debug){ fprintf(stderrfp, "enabled Interpolation\n"); }
                }
            }
            if (strcmp(key, "enable_1d") == 0) { 
                config->enable_1d=0;
                if (strcmp(value,"on") == 0) { config->enable_1d=1;
if(muscal_ucvm_debug){ fprintf(stderrfp, "enabled muscal1d to fill all empty returns\n"); }
                }
            }
         /* get data for the dataset */
            if (strcmp(key, "data_file") == 0) { 
                  int rc=_setup_a_dataset(config,value);
                  if(rc !=1 ) { muscal_print_error("BAD data_file."); }
         } 
        }
    }

    // Have we set up all muscal_configuration parameters?
    if (config->utm_zone == 0 || (strcmp(config->model_dir,"")==0) ||
        config->dataset_file == NULL  || config->dataset_label == NULL ) {
        muscal_print_error("One muscal_configuration parameter not specified. Please check your muscal_configuration file.");
        return UCVM_MODEL_CODE_ERROR;
    }

    fclose(fp);
    return UCVM_MODEL_CODE_SUCCESS;
}

/**
 * Extract muscal dataset specific info
 * and fill in the model info, one dataset at a time
 * and allocate required memory space
 *
 * @param 
 */
int _setup_a_dataset(muscal_configuration_t *config, char *blobstr) {
    /* parse the blob and grab the meta data */
    cJSON *confjson=cJSON_Parse(blobstr);

    /* grab the related file and extract dataset info */
    if(confjson == NULL) {
        const char *eptr = cJSON_GetErrorPtr();
        if(eptr != NULL) {
            if(muscal_ucvm_debug){ fprintf(stderrfp, "Configuration dataset setup error: (%s)\n", eptr); }
            return FAIL;
        }
    }

    cJSON *label = cJSON_GetObjectItemCaseSensitive(confjson, "LABEL");
    if(cJSON_IsString(label)){
        config->dataset_label=strdup(label->valuestring);
    }
    cJSON *file = cJSON_GetObjectItemCaseSensitive(confjson, "FILE");
    if(cJSON_IsString(file)){
        config->dataset_file=strdup(file->valuestring);
    }
    cJSON *surface = cJSON_GetObjectItemCaseSensitive(confjson, "SURFACE");
    if(cJSON_IsString(surface)){
        config->surface_file=strdup(surface->valuestring);
        } else {
            config->surface_file=NULL;
    }
    cJSON *surface_count = cJSON_GetObjectItemCaseSensitive(confjson, "SURFACE_COUNT");
    if(cJSON_IsNumber(surface_count)) {
        config->surface_count=surface_count->valueint;
        } else {
          config->surface_count=0;
    }
    return 1;
}

void _trimLast(char *str, char m) {
  int i;
  i = strlen(str);
  while (str[i-1] == m) {
    str[i-1] = '\0';
    i = i - 1;
  }
  return;
}

void _splitline(char* lptr, char key[], char value[]) {

  char *kptr, *vptr;

  for(int i=0; i<strlen(key); i++) { key[i]='\0'; }

  _trimLast(lptr,'\n');
  vptr = strchr(lptr, '=');
  int pos=vptr - lptr;

// skip space in key token from the back
  while ( lptr[pos-1]  == ' ') {
    pos--;
  }

  strncpy(key,lptr, pos);
  key[pos] = '\0';

  vptr++;
  while( vptr[0] == ' ' ) {
    vptr++;
  }
  strcpy(value,vptr);
  _trimLast(value,' ');
}

/**
 * Called to clear out the allocated memory 
 *
 * @param 
 */
int muscal_configuration_finalize(muscal_configuration_t *config) {
    free(config->dataset_label);  
    free(config->dataset_file);  
    if(config->surface_file !=NULL) free(config->surface_file);  
    free(config);
    return SUCCESS;
}

/**
 * Prints the error string provided.
 *
 * @param err The error string to print out to stderr.
 */
void muscal_print_error(char *err) {
    fprintf(stderr, "An error has occurred while executing muscal. The error was: \n\n%s\n",err);
    fprintf(stderr, "\nPlease contact software@scec.org and describe both the error and a bit\n");
    fprintf(stderr, "about the computer you are running muscal on (Linux, Mac, etc.).\n");
}

// The following functions are for dynamic library mode. If we are compiling
// a static library, these functions must be disabled to avoid conflicts.
#ifdef DYNAMIC_LIBRARY

/**
 * Init function loaded and called by the UCVM library. Calls muscal_init.
 *
 * @param dir The directory in which UCVM is installed.
 * @return Success or failure.
 */
int model_init(const char *dir, const char *label) {
    return muscal_init(dir, label);
}

/**
 * Query function loaded and called by the UCVM library. Calls muscal_query.
 *
 * @param points The basic_point_t array containing the points.
 * @param data The basic_properties_t array containing the material properties returned.
 * @param numpoints The number of points in the array.
 * @return Success or fail.
 */
int model_query(muscal_point_t *points, muscal_properties_t *data, int numpoints) {
    return muscal_query(points, data, numpoints);
}

/**
 * Finalize function loaded and called by the UCVM library. Calls muscal_finalize.
 *
 * @return Success
 */
int model_finalize() {
    return muscal_finalize();
}

/**
 * Version function loaded and called by the UCVM library. Calls muscal_version.
 *
 * @param ver Version string to return.
 * @param len Maximum length of buffer.
 * @return Zero
 */
int model_version(char *ver, int len) {
    return muscal_version(ver, len);
}

/**
 * Version function loaded and called by the UCVM library. Calls muscal_config.
 *
 * @param config Config string to return.
 * @param sz number of config terms
 * @return Zero
 */
int model_config(char **config, int *sz) {
    return muscal_config(config, sz);
}


int (*get_model_init())(const char *, const char *) {
        return &muscal_init;
}
int (*get_model_query())(muscal_point_t *, muscal_properties_t *, int) {
         return &muscal_query;
}
int (*get_model_finalize())() {
         return &muscal_finalize;
}
int (*get_model_version())(char *, int) {
         return &muscal_version;
}
int (*get_model_config())(char **, int*) {
    return &muscal_config;
}



#endif
