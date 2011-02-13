#ifndef XMI_DATA_STRUCTS_H
#define XMI_DATA_STRUCTS_H

#include <stdio.h>

struct xmi_general {
	float version;
	char *outputfile;
	long int n_photons_interval;
	long int n_photons_line;
	int n_interactions_trajectory;
};


struct xmi_layer {
	int n_elements;
	int *Z;
	double *weight;
	double density;
	double thickness;
};





struct xmi_composition {
	int n_layers;
	struct xmi_layer *layers;
	int reference_layer;
};


struct xmi_geometry {
	double d_sample_source;
	double n_sample_orientation[3];
	double p_detector_window[3];
	double n_detector_orientation[3];
	double area_detector;
	double acceptance_detector;
	double d_source_slit;
	double slit_size_x;
	double slit_size_y;
};

struct xmi_energy {
	double energy;
	double horizontal_intensity;
	double vertical_intensity;
	double sigma_x;
	double sigma_xp;
	double sigma_y;
	double sigma_yp;
};

struct xmi_excitation {
	int n_discrete;
	struct xmi_energy *discrete;
	int n_continuous;
	struct xmi_energy *continuous;
};


struct xmi_absorbers {
	int n_exc_layers;
	struct xmi_layer *exc_layers;
	int n_det_layers;
	struct xmi_layer *det_layers;
};

#define XMI_DETECTOR_SILI 0
#define XMI_DETECTOR_GE 1
#define XMI_DETECTOR_SI_SDD 2



struct xmi_detector {
	int detector_type;
	double gain;
	double zero;
	double fano;
	double noise;
	double max_convolution_energy;
	int n_crystal_layers;
	struct xmi_layer *crystal_layers;
};


struct xmi_input {
	struct xmi_general *general;
	struct xmi_composition *composition;
	struct xmi_geometry *geometry;
	struct xmi_excitation *excitation;
	struct xmi_absorbers *absorbers;
	struct xmi_detector *detector;
};


//typedefs are clearer then using void *...
//these correspond in a more transparent way with the Fortran variables
typedef void* xmi_inputFPtr;  
typedef void* xmi_hdf5FPtr;

#define XMI_COMPARE_GENERAL 1
#define XMI_COMPARE_COMPOSITION 2
#define XMI_COMPARE_GEOMETRY 4
#define XMI_COMPARE_EXCITATION 8
#define XMI_COMPARE_ABSORBERS 16
#define XMI_COMPARE_DETECTOR 32

// 0.01%
#define XMI_COMPARE_THRESHOLD 0.0001

void xmi_free_input(struct xmi_input *);

//returns 0 when identical, returns a number larger than 0 consisting of OR-ed XMI_COMPARE_* macros if not identical
int xmi_compare_input(struct xmi_input *A, struct xmi_input *B);

//returns 0 when ok
int xmi_validate_input(struct xmi_input *);

void xmi_copy_input(struct xmi_input *A, struct xmi_input **B);

void xmi_free_composition(struct xmi_composition *);

void xmi_copy_composition(struct xmi_composition *A, struct xmi_composition **B);

void xmi_free_layer (struct xmi_layer *layer); 

void xmi_copy_layer(struct xmi_layer *, struct xmi_layer **B); 
void xmi_copy_layer2(struct xmi_layer *, struct xmi_layer *B); 

struct xmi_input *xmi_init_empty_input(void);

void xmi_free_absorbers(struct xmi_absorbers *);

void xmi_copy_absorbers(struct xmi_absorbers *A, struct xmi_absorbers **B);

void xmi_copy_abs_or_crystal2composition(struct xmi_layer *layers, int n_layers, struct xmi_composition **composition); 

void xmi_copy_composition2abs_or_crystal(struct xmi_composition *composition, struct xmi_layer **layers, int *n_layers);


//Fortran function that copies a C xmi_input structure to the corresponding Fortran TYPE variable. The function returns a pointer to the memory locatie of the Fortran variable
void xmi_input_C2F(struct xmi_input *xmi_inputC, xmi_inputFPtr *Ptr );

//Fortran function that frees a Fortran xmi_input TYPE variable. The value of the pointer shall be set to NULL afterwards.
void xmi_free_input_F(xmi_inputFPtr *inputFPtr);

//Fortran function that reads in from the HDF5 data file what it needs... return 1 on success, 0 otherwise
int xmi_init_from_hdf5(char *hdf5_file, xmi_inputFPtr inputFPtr, xmi_hdf5FPtr *hdf5FPtr );

//Fortran function that frees a Fortran xmi_hdf5 TYPE variable. The value of the pointer shall be set to NULL afterwards.
void xmi_free_hdf5_F(xmi_hdf5FPtr *hdf5FPtr);

int xmi_update_input_from_hdf5(xmi_inputFPtr inputFPtr, xmi_hdf5FPtr hdf5FPtr);

//Fortran function that further initializes the input
int xmi_init_input(xmi_inputFPtr *inputFPtr);

//prints the contents of the structure... useful when debugging
void xmi_print_input(FILE *fPtr, struct xmi_input *input);

void xmi_print_layer(FILE *fPtr, struct xmi_layer *layer, int n_layers);


#endif
