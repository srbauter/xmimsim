/*
Copyright (C) 2010-2011 Tom Schoonjans and Laszlo Vincze

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "xmi_data_structs.h"
#include "xmi_aux.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


void xmi_free_layer (struct xmi_layer *layer) {
	free(layer->Z);
	free(layer->weight);
}





void xmi_free_input(struct xmi_input *input) {
	int i;

	//general
	free(input->general->outputfile);
	free(input->general->comments);
	free(input->general);

	//composition
	for (i = 0 ; i < input->composition->n_layers ; i++) 
		xmi_free_layer(input->composition->layers+i);
	 
	free(input->composition->layers);
	free(input->composition);

	//geometry
	free(input->geometry);

	//excitation
	if (input->excitation->n_discrete > 0)
		free(input->excitation->discrete);

	if (input->excitation->n_continuous > 0)
		free(input->excitation->continuous);

	free(input->excitation);

	//absorbers
	if (input->absorbers->n_exc_layers > 0) {
		for (i = 0 ; i < input->absorbers->n_exc_layers ; i++) 
			xmi_free_layer(input->absorbers->exc_layers+i);
		free(input->absorbers->exc_layers);
	}
	if (input->absorbers->n_det_layers > 0) {
		for (i = 0 ; i < input->absorbers->n_det_layers ; i++) 
			xmi_free_layer(input->absorbers->det_layers+i);
		free(input->absorbers->det_layers);
	}

	free(input->absorbers);

	//detector
	if (input->detector->n_crystal_layers > 0) {
		for (i = 0 ; i < input->detector->n_crystal_layers ; i++) 
			xmi_free_layer(input->detector->crystal_layers+i);
		free(input->detector->crystal_layers);
	}

	free(input->detector);

	//input
	free(input);


}


void xmi_copy_input(struct xmi_input *A, struct xmi_input **B) {
	int i;

	//allocate space for B
	*B = (struct xmi_input *) malloc(sizeof(struct xmi_input));

	//general
	//(*B)->general = (struct xmi_general *) malloc(sizeof(struct xmi_general));
	(*B)->general = (struct xmi_general *) xmi_memdup((A)->general, sizeof(struct xmi_general));
	(*B)->general->outputfile = strdup(A->general->outputfile);
	(*B)->general->comments= strdup(A->general->comments);

	//composition
	(*B)->composition = (struct xmi_composition *) malloc(sizeof(struct xmi_composition));
	(*B)->composition->n_layers = (A)->composition->n_layers;
	(*B)->composition->reference_layer = (A)->composition->reference_layer;
	(*B)->composition->layers = (struct xmi_layer *) xmi_memdup((A)->composition->layers,((A)->composition->n_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->composition->n_layers ; i++) {
		(*B)->composition->layers[i].Z = (int *) xmi_memdup((A)->composition->layers[i].Z,((A)->composition->layers[i].n_elements)*sizeof(int));
		(*B)->composition->layers[i].weight = (double *) xmi_memdup((A)->composition->layers[i].weight,((A)->composition->layers[i].n_elements)*sizeof(double));
	}
	
	//geometry
	(*B)->geometry = (struct xmi_geometry *) xmi_memdup((A)->geometry,sizeof(struct xmi_geometry));

	//excitation
	
	(*B)->excitation = (struct xmi_excitation *) malloc(sizeof(struct xmi_excitation));
	(*B)->excitation->n_discrete = (A)->excitation->n_discrete;
	(*B)->excitation->n_continuous = (A)->excitation->n_continuous;

	if ((*B)->excitation->n_discrete > 0) {
		(*B)->excitation->discrete = (struct xmi_energy *) xmi_memdup((A)->excitation->discrete,(A)->excitation->n_discrete*sizeof(struct xmi_energy));
	}
	else 
		(*B)->excitation->discrete = NULL;
	if ((*B)->excitation->n_continuous > 0) {
		(*B)->excitation->continuous = (struct xmi_energy *) xmi_memdup((A)->excitation->continuous,(A)->excitation->n_continuous*sizeof(struct xmi_energy));
	}
	else 
		(*B)->excitation->continuous = NULL;

	//absorbers
	(*B)->absorbers = (struct xmi_absorbers*) malloc(sizeof(struct xmi_absorbers));
	(*B)->absorbers->n_exc_layers = (A)->absorbers->n_exc_layers; 
	(*B)->absorbers->n_det_layers = (A)->absorbers->n_det_layers; 

	(*B)->absorbers->exc_layers = (struct xmi_layer *) xmi_memdup((A)->absorbers->exc_layers,((A)->absorbers->n_exc_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->absorbers->n_exc_layers ; i++) {
		(*B)->absorbers->exc_layers[i].Z = (int *) xmi_memdup((A)->absorbers->exc_layers[i].Z,((A)->absorbers->exc_layers[i].n_elements)*sizeof(int));
		(*B)->absorbers->exc_layers[i].weight = (double *) xmi_memdup((A)->absorbers->exc_layers[i].weight,((A)->absorbers->exc_layers[i].n_elements)*sizeof(double));
	}
	(*B)->absorbers->det_layers = (struct xmi_layer *) xmi_memdup((A)->absorbers->det_layers,((A)->absorbers->n_det_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->absorbers->n_det_layers ; i++) {
		(*B)->absorbers->det_layers[i].Z = (int *) xmi_memdup((A)->absorbers->det_layers[i].Z,((A)->absorbers->det_layers[i].n_elements)*sizeof(int));
		(*B)->absorbers->det_layers[i].weight = (double *) xmi_memdup((A)->absorbers->det_layers[i].weight,((A)->absorbers->det_layers[i].n_elements)*sizeof(double));
	}

	//detector
	(*B)->detector = (struct xmi_detector *) xmi_memdup((A)->detector,sizeof(struct xmi_detector));
	(*B)->detector->crystal_layers = (struct xmi_layer *) xmi_memdup((A)->detector->crystal_layers,((A)->detector->n_crystal_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->detector->n_crystal_layers ; i++) {
		(*B)->detector->crystal_layers[i].Z = (int *) xmi_memdup((A)->detector->crystal_layers[i].Z,((A)->detector->crystal_layers[i].n_elements)*sizeof(int));
		(*B)->detector->crystal_layers[i].weight = (double *) xmi_memdup((A)->detector->crystal_layers[i].weight,((A)->detector->crystal_layers[i].n_elements)*sizeof(double));
	}

	


}


int xmi_compare_input(struct xmi_input *A, struct xmi_input *B) {
	int rv;
	int i,j;
	double *temparr1;
	double *temparr2;
	
	//Yes, I know every textbook on programming says not to use the goto construct but I'm going to do it anyway :-)
	//Don't try this at home though!
	
	rv = 0;

	//general
	if (A->general->version != B->general->version) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}

	if (strcmp(A->general->outputfile,B->general->outputfile) != 0) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}

	if (A->general->n_photons_interval != B->general->n_photons_interval) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}

	if (A->general->n_photons_line != B->general->n_photons_line) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}

	if (A->general->n_interactions_trajectory != B->general->n_interactions_trajectory) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}

	if (strcmp(A->general->comments,B->general->comments) != 0) {
		rv |= XMI_COMPARE_GENERAL;
		goto after_general;
	}


	after_general:


	//composition
	if (A->composition->n_layers != B->composition->n_layers) {
		rv |= XMI_COMPARE_COMPOSITION;
	}
	else if (A->composition->reference_layer != B->composition->reference_layer) {
		rv |= XMI_COMPARE_COMPOSITION;
	}
	else {
		for (i = 0 ; i < A->composition->n_layers ; i++) {
			if (A->composition->layers[i].n_elements != B->composition->layers[i].n_elements) {
				rv |= XMI_COMPARE_COMPOSITION;
				break;
			}
			else {
				for (j = 0 ; j < A->composition->layers[i].n_elements ; j++) {
					if (A->composition->layers[i].Z[j] != B->composition->layers[i].Z[j]) {
						rv |= XMI_COMPARE_COMPOSITION;
						goto after_composition;	
					}	
					else if (fabsl(A->composition->layers[i].weight[j]- B->composition->layers[i].weight[j])/A->composition->layers[i].weight[j] >XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_COMPOSITION;
						goto after_composition;	
					}	
				} 
				if (fabsl(A->composition->layers[i].density - B->composition->layers[i].density)/A->composition->layers[i].density > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_COMPOSITION;
					break;
				}
				if (fabsl(A->composition->layers[i].thickness- B->composition->layers[i].thickness)/A->composition->layers[i].thickness > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_COMPOSITION;
					break;
				}
			}
		}
	}

	after_composition:

	//geometry
#define XMI_IF_COMPARE_GEOMETRY(a) if (fabsl(A->geometry->a - B->geometry->a)/fabs(A->geometry->a) > XMI_COMPARE_THRESHOLD){\
	rv |= XMI_COMPARE_GEOMETRY;\
	goto after_geometry;\
	}	
#define XMI_IF_COMPARE_GEOMETRY2(a) if (fabsl(A->geometry->a - B->geometry->a) > XMI_COMPARE_THRESHOLD){\
	rv |= XMI_COMPARE_GEOMETRY;\
	goto after_geometry;\
	}	

#define XMI_IF_COMPARE_GEOMETRY3(a,b) if (fabsl(a - b) > XMI_COMPARE_THRESHOLD){\
	rv |= XMI_COMPARE_GEOMETRY;\
	goto after_geometry;\
	}	

	XMI_IF_COMPARE_GEOMETRY(d_sample_source)
	//should compare normalized orientations...
	temparr1 = (double *) xmi_memdup(A->geometry->n_sample_orientation,sizeof(double)*3);
	temparr2 = (double *) xmi_memdup(B->geometry->n_sample_orientation,sizeof(double)*3);
	xmi_normalize_vector_double(temparr1, 3);
	xmi_normalize_vector_double(temparr2, 3);

	XMI_IF_COMPARE_GEOMETRY3(temparr1[0],temparr2[0])
	XMI_IF_COMPARE_GEOMETRY3(temparr1[1],temparr2[1])
	XMI_IF_COMPARE_GEOMETRY3(temparr1[2],temparr2[2])
	free(temparr1);
	free(temparr2);
	XMI_IF_COMPARE_GEOMETRY2(p_detector_window[0])
	XMI_IF_COMPARE_GEOMETRY2(p_detector_window[1])
	XMI_IF_COMPARE_GEOMETRY2(p_detector_window[2])
	//should compare normalized orientations...
	temparr1 = (double *) xmi_memdup(A->geometry->n_detector_orientation,sizeof(double)*3);
	temparr2 = (double *) xmi_memdup(B->geometry->n_detector_orientation,sizeof(double)*3);
	xmi_normalize_vector_double(temparr1, 3);
	xmi_normalize_vector_double(temparr2, 3);
	XMI_IF_COMPARE_GEOMETRY3(temparr1[0],temparr2[0])
	XMI_IF_COMPARE_GEOMETRY3(temparr1[1],temparr2[1])
	XMI_IF_COMPARE_GEOMETRY3(temparr1[2],temparr2[2])
	free(temparr1);
	free(temparr2);
	XMI_IF_COMPARE_GEOMETRY(area_detector)
	XMI_IF_COMPARE_GEOMETRY2(collimator_height)
	XMI_IF_COMPARE_GEOMETRY2(collimator_diameter)
	XMI_IF_COMPARE_GEOMETRY2(d_source_slit)
	XMI_IF_COMPARE_GEOMETRY2(slit_size_x)
	XMI_IF_COMPARE_GEOMETRY2(slit_size_y)

	after_geometry:

#define XMI_IF_COMPARE_EXCITATION_DISCRETE(a) if (fabsl(A->excitation->discrete[i].a-B->excitation->discrete[i].a)/A->excitation->discrete[i].a > XMI_COMPARE_THRESHOLD) {\
					rv |= XMI_COMPARE_EXCITATION;\
					break;\
				}

	//excitation
	if (A->excitation->n_discrete > 0 || B->excitation->n_discrete > 0) {
		if (A->excitation->n_discrete != B->excitation->n_discrete) {
			rv |= XMI_COMPARE_EXCITATION;
		}
		else {
			for (i = 0 ; i < A->excitation->n_discrete ; i++) {
				XMI_IF_COMPARE_EXCITATION_DISCRETE(energy)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(horizontal_intensity)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(vertical_intensity)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(sigma_x)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(sigma_xp)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(sigma_y)
				XMI_IF_COMPARE_EXCITATION_DISCRETE(sigma_yp)
			}
		}
	}	

#define XMI_IF_COMPARE_EXCITATION_CONTINUOUS(a) if (fabsl(A->excitation->continuous[i].a-B->excitation->continuous[i].a)/A->excitation->continuous[i].a > XMI_COMPARE_THRESHOLD) {\
					rv |= XMI_COMPARE_EXCITATION;\
					break;\
				}
	if (A->excitation->n_continuous > 0 || B->excitation->n_continuous > 0) {
		if (A->excitation->n_continuous != B->excitation->n_continuous) {
			rv |= XMI_COMPARE_EXCITATION;
		}
		else {
			for (i = 0 ; i < A->excitation->n_continuous ; i++) {
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(energy)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(horizontal_intensity)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(vertical_intensity)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(sigma_x)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(sigma_xp)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(sigma_y)
				XMI_IF_COMPARE_EXCITATION_CONTINUOUS(sigma_yp)
			}
		}
	}	

	//absorbers
	if (A->absorbers->n_exc_layers > 0 || B->absorbers->n_exc_layers > 0) {
		if (A->absorbers->n_exc_layers != B->absorbers->n_exc_layers) {
			rv |= XMI_COMPARE_ABSORBERS;
		}
		else {
			for (i = 0 ; i < A->absorbers->n_exc_layers ; i++) {
				if (A->absorbers->exc_layers[i].n_elements != B->absorbers->exc_layers[i].n_elements) {
					rv |= XMI_COMPARE_ABSORBERS;
					break;
				}
				else {
					for (j = 0 ; j < A->absorbers->exc_layers[i].n_elements ; j++) {
						if (A->absorbers->exc_layers[i].Z[j] != B->absorbers->exc_layers[i].Z[j]) {
							rv |= XMI_COMPARE_ABSORBERS;
							goto after_absorbers;	
						}	
						else if (fabsl(A->absorbers->exc_layers[i].weight[j]- B->absorbers->exc_layers[i].weight[j])/A->absorbers->exc_layers[i].weight[j] >XMI_COMPARE_THRESHOLD) {
							rv |= XMI_COMPARE_ABSORBERS;
							goto after_absorbers;	
						}	
					} 
					if (fabsl(A->absorbers->exc_layers[i].density - B->absorbers->exc_layers[i].density)/A->absorbers->exc_layers[i].density > XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_ABSORBERS;
						break;
					}
					if (fabsl(A->absorbers->exc_layers[i].thickness- B->absorbers->exc_layers[i].thickness)/A->absorbers->exc_layers[i].thickness > XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_ABSORBERS;
						break;
					}
				}
			}
		}
	}
	if (A->absorbers->n_det_layers > 0 || B->absorbers->n_det_layers > 0) {
		if (A->absorbers->n_det_layers != B->absorbers->n_det_layers) {
			rv |= XMI_COMPARE_ABSORBERS;
		}
		else {
			for (i = 0 ; i < A->absorbers->n_det_layers ; i++) {
				if (A->absorbers->det_layers[i].n_elements != B->absorbers->det_layers[i].n_elements) {
					rv |= XMI_COMPARE_ABSORBERS;
					break;
				}
				else {
					for (j = 0 ; j < A->absorbers->det_layers[i].n_elements ; j++) {
						if (A->absorbers->det_layers[i].Z[j] != B->absorbers->det_layers[i].Z[j]) {
							rv |= XMI_COMPARE_ABSORBERS;
							goto after_absorbers;	
						}	
						else if (fabsl(A->absorbers->det_layers[i].weight[j]- B->absorbers->det_layers[i].weight[j])/A->absorbers->det_layers[i].weight[j] >XMI_COMPARE_THRESHOLD) {
							rv |= XMI_COMPARE_ABSORBERS;
							goto after_absorbers;	
						}	
					} 
					if (fabsl(A->absorbers->det_layers[i].density - B->absorbers->det_layers[i].density)/A->absorbers->det_layers[i].density > XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_ABSORBERS;
						break;
					}
					if (fabsl(A->absorbers->det_layers[i].thickness- B->absorbers->det_layers[i].thickness)/A->absorbers->det_layers[i].thickness > XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_ABSORBERS;
						break;
					}
				}
			}
		}
	}

	after_absorbers:

	//detector
	if (A->detector->detector_type != B->detector->detector_type) {
		rv |= XMI_COMPARE_DETECTOR;
		goto after_detector;
	}

#define XMI_IF_COMPARE_DETECTOR(a) if (fabsl(A->detector->a - B->detector->a)/A->detector->a > XMI_COMPARE_THRESHOLD){\
	rv |= XMI_COMPARE_DETECTOR;\
	goto after_detector;\
	}

	XMI_IF_COMPARE_DETECTOR(live_time)
	XMI_IF_COMPARE_DETECTOR(pulse_width)
	XMI_IF_COMPARE_DETECTOR(gain)
	XMI_IF_COMPARE_DETECTOR(zero)
	XMI_IF_COMPARE_DETECTOR(fano)
	XMI_IF_COMPARE_DETECTOR(noise)
	XMI_IF_COMPARE_DETECTOR(max_convolution_energy)

	if (A->detector->n_crystal_layers != B->detector->n_crystal_layers) {
		rv |= XMI_COMPARE_DETECTOR;
	}
	else {
		for (i = 0 ; i < A->detector->n_crystal_layers ; i++) {
			if (A->detector->crystal_layers[i].n_elements != B->detector->crystal_layers[i].n_elements) {
				rv |= XMI_COMPARE_DETECTOR;
				break;
			}
			else {
				for (j = 0 ; j < A->detector->crystal_layers[i].n_elements ; j++) {
					if (A->detector->crystal_layers[i].Z[j] != B->detector->crystal_layers[i].Z[j]) {
						rv |= XMI_COMPARE_DETECTOR;
						goto after_detector;	
					}	
					else if (fabsl(A->detector->crystal_layers[i].weight[j]- B->detector->crystal_layers[i].weight[j])/A->detector->crystal_layers[i].weight[j] >XMI_COMPARE_THRESHOLD) {
						rv |= XMI_COMPARE_DETECTOR;
						goto after_detector;	
					}	
				} 
				if (fabsl(A->detector->crystal_layers[i].density - B->detector->crystal_layers[i].density)/A->detector->crystal_layers[i].density > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_DETECTOR;
					break;
				}
				if (fabsl(A->detector->crystal_layers[i].thickness- B->detector->crystal_layers[i].thickness)/A->detector->crystal_layers[i].thickness > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_DETECTOR;
					break;
				}
			}
		}
	}



	after_detector:

	return rv;


}

void xmi_free_composition(struct xmi_composition *composition) {
	int i;

	for (i = 0 ; i < composition->n_layers ; i++) 
		xmi_free_layer(composition->layers+i);
	 
	free(composition->layers);

	free(composition);
}

void xmi_copy_composition(struct xmi_composition *A, struct xmi_composition **B) {
	int i;

	//allocate space for B
	*B = (struct xmi_composition *) malloc(sizeof(struct xmi_composition));
	(*B)->n_layers = A->n_layers;
	(*B)->reference_layer = A->reference_layer;
	(*B)->layers = (struct xmi_layer *) xmi_memdup((A)->layers,((A)->n_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->n_layers ; i++) {
		(*B)->layers[i].Z = (int *) xmi_memdup((A)->layers[i].Z,((A)->layers[i].n_elements)*sizeof(int));
		(*B)->layers[i].weight = (double *) xmi_memdup((A)->layers[i].weight,((A)->layers[i].n_elements)*sizeof(double));
	}


}

void xmi_copy_layer(struct xmi_layer *A, struct xmi_layer **B) {
	//allocate space for B
	*B = (struct xmi_layer *) malloc(sizeof(struct xmi_layer));
	(*B)->n_elements = A->n_elements;
	(*B)->density = A->density;
	(*B)->thickness = A->thickness;
	(*B)->Z = (int *) xmi_memdup(A->Z, A->n_elements*sizeof(int));
	(*B)->weight = (double*) xmi_memdup(A->weight, A->n_elements*sizeof(double));
}


void xmi_copy_layer2(struct xmi_layer *A, struct xmi_layer *B) {
	B->n_elements = A->n_elements;
	B->density = A->density;
	B->thickness = A->thickness;
	B->Z = (int *) xmi_memdup(A->Z, A->n_elements*sizeof(int));
	B->weight = (double*) xmi_memdup(A->weight, A->n_elements*sizeof(double));
}

struct xmi_input *xmi_init_empty_input(void) {

	struct xmi_input *rv;

	rv = (struct xmi_input *) malloc(sizeof(struct xmi_input));

	//general
	rv->general = (struct xmi_general *) malloc(sizeof(struct xmi_general));
	rv->general->version = 1.0;
	rv->general->outputfile = strdup("");
	rv->general->comments= strdup("");
	rv->general->n_photons_interval = 10000;
	rv->general->n_photons_line = 100000;
	rv->general->n_interactions_trajectory = 4;

	//layer
	rv->composition = (struct xmi_composition *) malloc(sizeof(struct xmi_composition));
	rv->composition->n_layers = 0;
	rv->composition->layers = NULL;
	rv->composition->reference_layer = 1;

	//geometry
	rv->geometry = (struct xmi_geometry *) malloc(sizeof(struct xmi_geometry));
	rv->geometry->d_sample_source=100.0;
	rv->geometry->n_sample_orientation[0] = 0.0;
	rv->geometry->n_sample_orientation[1] = sqrt(2.0)/2.0;
	rv->geometry->n_sample_orientation[2] = sqrt(2.0)/2.0;
	rv->geometry->p_detector_window[0] = 0.0;
	rv->geometry->p_detector_window[1] = -1.0;
	rv->geometry->p_detector_window[2] = 100.0;
	rv->geometry->n_detector_orientation[0] = 0.0;
	rv->geometry->n_detector_orientation[1] = 1.0;
	rv->geometry->n_detector_orientation[2] = 0.0;
	rv->geometry->area_detector = 0.3;
	//default is NO collimator
	rv->geometry->collimator_height = 0.0;
	rv->geometry->collimator_diameter= 0.0;
	rv->geometry->d_source_slit = 100.0;
	rv->geometry->slit_size_x = 0.001;
	rv->geometry->slit_size_y = 0.001;

	//excitation
	rv->excitation = (struct xmi_excitation *) malloc(sizeof(struct xmi_excitation));
	rv->excitation->n_discrete = 1;
	rv->excitation->n_continuous = 0;
	rv->excitation->continuous = NULL;
	rv->excitation->discrete = (struct xmi_energy *) malloc(sizeof(struct xmi_energy));
	rv->excitation->discrete[0].energy = 28.0;
	rv->excitation->discrete[0].horizontal_intensity= 1E12;
	rv->excitation->discrete[0].vertical_intensity= 1E9;
	rv->excitation->discrete[0].sigma_x= 0.0;
	rv->excitation->discrete[0].sigma_xp= 0.0;
	rv->excitation->discrete[0].sigma_y= 0.0;
	rv->excitation->discrete[0].sigma_yp= 0.0;

	//absorbers
	rv->absorbers = (struct xmi_absorbers *) malloc(sizeof(struct xmi_absorbers));
	rv->absorbers->n_exc_layers = 0;
	rv->absorbers->exc_layers = NULL;
	rv->absorbers->n_det_layers = 1;
	rv->absorbers->det_layers = malloc(sizeof(struct xmi_layer));
	rv->absorbers->det_layers[0].n_elements = 1;
	rv->absorbers->det_layers[0].Z = (int *) malloc(sizeof(int));
	rv->absorbers->det_layers[0].weight = (double *) malloc(sizeof(double));
	rv->absorbers->det_layers[0].Z[0] = 4;
	rv->absorbers->det_layers[0].weight[0] = 1.0;
	rv->absorbers->det_layers[0].density = 1.85;
	rv->absorbers->det_layers[0].thickness = 0.002;

	//detector
	rv->detector = (struct xmi_detector *) malloc(sizeof(struct xmi_detector));
	rv->detector->detector_type = XMI_DETECTOR_SILI;
	rv->detector->live_time = 1;
	rv->detector->pulse_width= 10E-6;
	rv->detector->gain = 20.0/1000.0;
	rv->detector->zero = 0.0;
	rv->detector->fano = 0.12;
	rv->detector->noise = 0.1;
	rv->detector->max_convolution_energy = 40.0;
	rv->detector->n_crystal_layers = 1;
	rv->detector->crystal_layers = malloc(sizeof(struct xmi_layer));
	rv->detector->crystal_layers[0].n_elements = 1;
	rv->detector->crystal_layers[0].Z = (int *) malloc(sizeof(int));
	rv->detector->crystal_layers[0].weight = (double *) malloc(sizeof(double));
	rv->detector->crystal_layers[0].Z[0] = 14;
	rv->detector->crystal_layers[0].weight[0] = 1.0;
	rv->detector->crystal_layers[0].density = 2.33;
	rv->detector->crystal_layers[0].thickness = 0.5;



	return rv;

}

void xmi_free_absorbers(struct xmi_absorbers *A) {
	int i;

	for (i = 0 ; i < A->n_exc_layers ; i++) 
		xmi_free_layer(A->exc_layers+i);
	 
	free(A->exc_layers);

	for (i = 0 ; i < A->n_det_layers ; i++) 
		xmi_free_layer(A->det_layers+i);
	 
	free(A->det_layers);

	free(A);

}

void xmi_copy_absorbers(struct xmi_absorbers *A, struct xmi_absorbers **B) {
	int i;

	//allocate space for B
	*B = (struct xmi_absorbers *) malloc(sizeof(struct xmi_absorbers));
	(*B)->n_exc_layers = A->n_exc_layers;
	(*B)->exc_layers = (struct xmi_layer *) xmi_memdup((A)->exc_layers,((A)->n_exc_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->n_exc_layers ; i++) {
		(*B)->exc_layers[i].Z = (int *) xmi_memdup((A)->exc_layers[i].Z,((A)->exc_layers[i].n_elements)*sizeof(int));
		(*B)->exc_layers[i].weight = (double *) xmi_memdup((A)->exc_layers[i].weight,((A)->exc_layers[i].n_elements)*sizeof(double));
	}
	
	(*B)->n_det_layers = A->n_det_layers;
	(*B)->det_layers = (struct xmi_layer *) xmi_memdup((A)->det_layers,((A)->n_det_layers)*sizeof(struct xmi_layer));
	for (i = 0 ; i < (A)->n_det_layers ; i++) {
		(*B)->det_layers[i].Z = (int *) xmi_memdup((A)->det_layers[i].Z,((A)->det_layers[i].n_elements)*sizeof(int));
		(*B)->det_layers[i].weight = (double *) xmi_memdup((A)->det_layers[i].weight,((A)->det_layers[i].n_elements)*sizeof(double));
	}

}

void xmi_copy_abs_or_crystal2composition(struct xmi_layer *layers, int n_layers, struct xmi_composition **composition) {
	int i;

	*composition = (struct xmi_composition *) malloc(sizeof(struct xmi_composition));
	(*composition)->n_layers = n_layers;
	if (n_layers > 0) {
		(*composition)->layers = (struct xmi_layer *) malloc(sizeof(struct xmi_layer)*n_layers); 
		for (i = 0 ; i < n_layers ; i++) 
			xmi_copy_layer2(layers+i,(*composition)->layers+i);
	}
	else 
		(*composition)->layers = NULL;

}

void xmi_copy_composition2abs_or_crystal(struct xmi_composition *composition, struct xmi_layer **layers, int *n_layers) {
	int i;
	
	*n_layers = composition->n_layers;

	if (*n_layers > 0) {
		*layers	= (struct xmi_layer *) malloc(sizeof(struct xmi_layer)**n_layers);
		for (i = 0 ; i < *n_layers ; i++) {
			xmi_copy_layer2(composition->layers+i, (*layers)+i);
		}
	}
	else
		*layers = NULL;

	return;
}

int xmi_validate_input(struct xmi_input *a) {
	int i,j;



	//validate general
	if (a->general->n_photons_interval <= 0) 
		return 1;

	if (a->general->n_photons_line <= 0) 
		return 1;

	if (a->general->n_interactions_trajectory <= 0) 
		return 1;

	if (strlen(a->general->outputfile) == 0)
		return 1;

	//composition
	if (a->composition->n_layers < 1)
		return 1;

	if (a->composition->reference_layer < 1 || a->composition->reference_layer > a->composition->n_layers)
		return 1;

	for (i = 0 ; i < a->composition->n_layers ; i++) {
			if (a->composition->layers[i].density <= 0.0)
				return 1;
			if (a->composition->layers[i].thickness <= 0.0)
				return 1;
	}

	//geometry
	if (a->geometry->d_sample_source <= 0.0)
		return 1;
	if (a->geometry->area_detector <= 0.0)
		return 1;
	if (a->geometry->collimator_height < 0.0)
		return 1;
	if (a->geometry->collimator_diameter < 0.0)
		return 1;
	

	if (a->excitation->n_discrete < 1 && a->excitation->n_continuous < 2)
		return 1;
	if (a->excitation->n_discrete ==0  && a->excitation->n_continuous < 2)
		return 1;


	//absorbers
	for (i = 0 ; i < a->absorbers->n_exc_layers ; i++) {
			if (a->absorbers->exc_layers[i].density <= 0.0)
				return 1;
			if (a->absorbers->exc_layers[i].thickness <= 0.0)
				return 1;
	}

	for (i = 0 ; i < a->absorbers->n_det_layers ; i++) {
			if (a->absorbers->det_layers[i].density <= 0.0)
				return 1;
			if (a->absorbers->det_layers[i].thickness <= 0.0)
				return 1;
	}

	//crystal
	if (a->detector->live_time <= 0.0) 
		return 1;
	if (a->detector->pulse_width <= 0.0)
		return 1;

	if (a->detector->n_crystal_layers < 1)
		return 1;

	for (i = 0 ; i < a->detector->n_crystal_layers ; i++) {
			if (a->detector->crystal_layers[i].density <= 0.0)
				return 1;
			if (a->detector->crystal_layers[i].thickness <= 0.0)
				return 1;
	}

	return 0;
}

void xmi_print_layer(FILE *fPtr ,struct xmi_layer *layer, int n_layers) {

	int i,j;


	for (i = 0 ; i < n_layers ; i++) {
		fprintf(fPtr,"Layer %i\n", i);
		for (j = 0 ; j < layer[i].n_elements ; j++) {
			fprintf(fPtr, "Z: %i -> weight: %lf\n",layer[i].Z[j],layer[i].weight[j]);
		}
		fprintf(fPtr, "density: %lf\n",layer[i].density);
		fprintf(fPtr, "thickness: %lf\n",layer[i].thickness);
	}
	return;
}

void xmi_print_input(FILE *fPtr, struct xmi_input *input) {
	int i, j;

	//general
	fprintf(fPtr, "general\n");
	fprintf(fPtr, "outputfile: %s\n",input->general->outputfile);
	fprintf(fPtr, "comments: %s\n",input->general->comments);
	fprintf(fPtr, "n_photons_interval: %li\n", input->general->n_photons_interval);
	fprintf(fPtr, "n_photons_line: %li\n", input->general->n_photons_line);
	fprintf(fPtr, "n_interactions_trajectory: %i\n", input->general->n_interactions_trajectory);
	fprintf(fPtr, "\n");

	//composition
	fprintf(fPtr, "composition\n");
	xmi_print_layer(fPtr, input->composition->layers, input->composition->n_layers);
	fprintf(fPtr, "reference_layer: %i\n",input->composition->reference_layer);
	fprintf(fPtr, "\n");

	//geometry
	fprintf(fPtr, "geometry\n");
	fprintf(fPtr, "d_sample_source: %lf\n", input->geometry->d_sample_source);
	fprintf(fPtr, "n_sample_orientation: %lf  %lf  %lf\n",input->geometry->n_sample_orientation[0],input->geometry->n_sample_orientation[1],input->geometry->n_sample_orientation[2]);
	fprintf(fPtr, "p_detector_window: %lf  %lf  %lf\n",input->geometry->p_detector_window[0],input->geometry->p_detector_window[1],input->geometry->p_detector_window[2]);
	fprintf(fPtr, "n_detector_orientation: %lf  %lf  %lf\n",input->geometry->n_detector_orientation[0],input->geometry->n_detector_orientation[1],input->geometry->n_detector_orientation[2]);
	fprintf(fPtr, "area_detector: %lf\n",input->geometry->area_detector);
	fprintf(fPtr, "collimator_height: %lf\n",input->geometry->collimator_height);
	fprintf(fPtr, "collimator_diameter: %lf\n",input->geometry->collimator_diameter);
	fprintf(fPtr, "d_source_slit: %lf\n",input->geometry->d_source_slit);
	fprintf(fPtr, "slit_size_x: %lf\n",input->geometry->slit_size_x);
	fprintf(fPtr, "slit_size_y: %lf\n",input->geometry->slit_size_y);
	fprintf(fPtr, "\n");

	//excitation
	fprintf(fPtr, "excitation\n");
	fprintf(fPtr, "discrete\n");
	for (i = 0 ; i < input->excitation->n_discrete ; i++) {
		fprintf(fPtr, "Energy %i: %lf\n",i,input->excitation->discrete[i].energy);
		fprintf(fPtr, "Horizontal intensity: %lf\n",input->excitation->discrete[i].horizontal_intensity);
		fprintf(fPtr, "Vertical intensity: %lf\n",input->excitation->discrete[i].vertical_intensity);
		fprintf(fPtr, "sigma_x: %lf\n",input->excitation->discrete[i].sigma_x);
		fprintf(fPtr, "sigma_xp: %lf\n",input->excitation->discrete[i].sigma_xp);
		fprintf(fPtr, "sigma_y: %lf\n",input->excitation->discrete[i].sigma_y);
		fprintf(fPtr, "sigma_yp: %lf\n",input->excitation->discrete[i].sigma_yp);
	}

	fprintf(fPtr, "continuous\n");
	for (i = 0 ; i < input->excitation->n_continuous ; i++) {
		fprintf(fPtr, "Energy %i: %lf\n",i,input->excitation->continuous[i].energy);
		fprintf(fPtr, "Horizontal intensity: %lf\n",input->excitation->continuous[i].horizontal_intensity);
		fprintf(fPtr, "Vertical intensity: %lf\n",input->excitation->continuous[i].vertical_intensity);
		fprintf(fPtr, "sigma_x: %lf\n",input->excitation->continuous[i].sigma_x);
		fprintf(fPtr, "sigma_xp: %lf\n",input->excitation->continuous[i].sigma_xp);
		fprintf(fPtr, "sigma_y: %lf\n",input->excitation->continuous[i].sigma_y);
		fprintf(fPtr, "sigma_yp: %lf\n",input->excitation->continuous[i].sigma_yp);
	}
	fprintf(fPtr, "\n");

	//absorbers
	fprintf(fPtr, "Beam absorbers\n");
	xmi_print_layer(fPtr, input->absorbers->exc_layers, input->absorbers->n_exc_layers);
	fprintf(fPtr, "Detector absorbers\n");
	xmi_print_layer(fPtr, input->absorbers->det_layers, input->absorbers->n_det_layers);
	fprintf(fPtr, "\n");

	//detector
	fprintf(fPtr, "Detector\n");
	fprintf(fPtr, "detectortype: %i\n",input->detector->detector_type);
	fprintf(fPtr, "gain: %lf\n", input->detector->gain);
	fprintf(fPtr, "live_time: %lf\n", input->detector->live_time);
	fprintf(fPtr, "pulse_width: %lf\n", input->detector->pulse_width);
	fprintf(fPtr, "zero: %lf\n", input->detector->zero);
	fprintf(fPtr, "fano: %lf\n", input->detector->fano);
	fprintf(fPtr, "noise: %lf\n", input->detector->noise);
	fprintf(fPtr, "max_convolution_energy: %lf\n", input->detector->max_convolution_energy);
	fprintf(fPtr, "detector crystal\n");
	xmi_print_layer(fPtr, input->detector->crystal_layers, input->detector->n_crystal_layers);
	fprintf(fPtr, "\n");



} 


