/**
 * @file muscal.h
 * @brief Main header file for MUSCAL library.
 * @version 1.0
 *
 * Delivers the MUSCAL model 
 *
 */
#ifndef MUSCAL_H
#define MUSCAL_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "muscal_dataset.h"
#include "muscal_util.h"

/** Defines a return value of success */
#define SUCCESS 0
/** Defines a return value of failure */
#define FAIL 1

/* config string */
#define MUSCAL_CONFIG_MAX 1000
#define MUSCAL_DATASET_MAX 1

extern int muscal_ucvm_debug;
extern int muscal_ucvm_debug_detail;
extern FILE *stderrfp;

// Structures
/** Defines a point (latitude, longitude, and depth) in WGS84 format */
typedef struct muscal_point_t {
	/** Longitude member of the point */
	double longitude;
	/** Latitude member of the point */
	double latitude;
	/** Depth member of the point */
	double depth;
} muscal_point_t;

/** Defines the material properties this model will retrieve. */
typedef struct muscal_properties_t {
	/** P-wave velocity in meters per second */
	double vp;
	/** S-wave velocity in meters per second */
	double vs;
	/** Density in g/m^3 */
	double rho;
	/** Qp */
	double qp;
	/** Qs */
	double qs;
} muscal_properties_t;

/** The MUSCAL configuration structure. */
typedef struct muscal_configuration_t {
	/** The zone of UTM projection */
	int utm_zone;
	/** The model directory */
	char model_dir[128];

	/** interpolation on or off (1 or 0) */
	int interpolation;

	/** add 1d on or off (1 or 0) */
	int enable_1d;

        /* how many datasets are in the model */
        char *dataset_file;  //strdup
	char *dataset_label; // strdup
        char *surface_file;  //strdup
        int surface_count;
} muscal_configuration_t;

// Constants
/** The version of the model. */
extern const char *muscal_version_string;

// Variables
/** Set to 1 when the model is ready for query. */
extern int muscal_is_initialized;

/** Configuration parameters. */
extern muscal_configuration_t *muscal_configuration;

/** Holds pointers to the velocity model data. */
extern muscal_dataset_t *muscal_dataset;

// UCVM API Required Functions

#ifdef DYNAMIC_LIBRARY

/** Initializes the model */
int model_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int model_finalize();
/** Returns version information */
int model_version(char *ver, int len);
/** Queries the model */
int model_query(muscal_point_t *points, muscal_properties_t *data, int numpts);

int (*get_model_init())(const char *, const char *);
int (*get_model_query())(muscal_point_t *, muscal_properties_t *, int);
int (*get_model_finalize())();
int (*get_model_version())(char *, int);

#endif

// MUSCAL Related Functions

/** Initializes the model */
int muscal_init(const char *dir, const char *label);
/** Cleans up the model (frees memory, etc.) */
int muscal_finalize();
/** Returns version information */
int muscal_version(char *ver, int len);
/** Queries the model */
int muscal_query(muscal_point_t *points, muscal_properties_t *data, int numpts);

// Non-UCVM Helper Functions
//
/** Reads the configuration file and helper functions. */
int muscal_read_configuration(char *file, muscal_configuration_t *config);
int muscal_configuration_finalize(muscal_configuration_t *config);

/** Prints out the error string. */
void muscal_print_error(char *err);
/** toggle debug flag **/
void muscal_setdebug();

/** parse JSON metadata blob per dataset **/
int _setup_a_dataset(muscal_configuration_t *conf, char *blobstr);

void _trimLast(char *str, char m);
void _splitline(char* lptr, char key[], char value[]);

#endif
