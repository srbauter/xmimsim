#include "xmi_data_structs.h"
#include "xmi_aux.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>


static void xmi_free_layer (struct xmi_layer *layer) {
	free(layer->Z);
	free(layer->weight);
}





void xmi_free_input(struct xmi_input *input) {
	int i;

	//general
	free(input->general->outputfile);
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
	if (input->absorbers->n_exc_layers > 0)
		for (i = 0 ; i < input->absorbers->n_exc_layers ; i++) 
			xmi_free_layer(input->absorbers->exc_layers+i);
		
	if (input->absorbers->n_det_layers > 0)
		for (i = 0 ; i < input->absorbers->n_det_layers ; i++) 
			xmi_free_layer(input->absorbers->det_layers+i);

	free(input->absorbers);

	//detector
	if (input->detector->n_crystal_layers > 0)	
		for (i = 0 ; i < input->detector->n_crystal_layers ; i++) 
			xmi_free_layer(input->detector->crystal_layers+i);

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

	//composition
	(*B)->composition = (struct xmi_composition *) malloc(sizeof(struct xmi_composition));
	(*B)->composition->n_layers = (A)->composition->n_layers;
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
	if ((*B)->excitation->n_continuous > 0) {
		(*B)->excitation->continuous = (struct xmi_energy *) xmi_memdup((A)->excitation->continuous,(A)->excitation->n_continuous*sizeof(struct xmi_energy));
	}

	//absorbers
	(*B)->absorbers = (struct xmi_absorbers*) malloc(sizeof(struct xmi_absorbers));
	(*B)->absorbers->n_exc_layers = (A)->absorbers->n_exc_layers; 
	(*B)->absorbers->n_det_layers = (A)->absorbers->n_det_layers; 

	if ((*B)->absorbers->n_exc_layers > 0) {
		(*B)->absorbers->exc_layers = (struct xmi_layer *) xmi_memdup((A)->absorbers->exc_layers,((A)->absorbers->n_exc_layers)*sizeof(struct xmi_layer));
		for (i = 0 ; i < (A)->absorbers->n_exc_layers ; i++) {
			(*B)->absorbers->exc_layers[i].Z = (int *) xmi_memdup((A)->absorbers->exc_layers[i].Z,((A)->absorbers->exc_layers[i].n_elements)*sizeof(int));
			(*B)->absorbers->exc_layers[i].weight = (double *) xmi_memdup((A)->absorbers->exc_layers[i].weight,((A)->absorbers->exc_layers[i].n_elements)*sizeof(double));
		}
	}
	if ((*B)->absorbers->n_det_layers > 0) {
		(*B)->absorbers->det_layers = (struct xmi_layer *) xmi_memdup((A)->absorbers->det_layers,((A)->absorbers->n_det_layers)*sizeof(struct xmi_layer));
		for (i = 0 ; i < (A)->absorbers->n_det_layers ; i++) {
			(*B)->absorbers->det_layers[i].Z = (int *) xmi_memdup((A)->absorbers->det_layers[i].Z,((A)->absorbers->det_layers[i].n_elements)*sizeof(int));
			(*B)->absorbers->det_layers[i].weight = (double *) xmi_memdup((A)->absorbers->det_layers[i].weight,((A)->absorbers->det_layers[i].n_elements)*sizeof(double));
		}
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

	after_general:


	//composition
	if (A->composition->n_layers != B->composition->n_layers) {
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
#define XMI_IF_COMPARE_GEOMETRY(a) if (fabsl(A->geometry->a - B->geometry->a)/A->geometry->a > XMI_COMPARE_THRESHOLD){\
	rv |= XMI_COMPARE_GEOMETRY;\
	goto after_geometry;\
	}	

	XMI_IF_COMPARE_GEOMETRY(d_sample_source)
	XMI_IF_COMPARE_GEOMETRY(n_sample_orientation[0])
	XMI_IF_COMPARE_GEOMETRY(n_sample_orientation[1])
	XMI_IF_COMPARE_GEOMETRY(n_sample_orientation[2])
	XMI_IF_COMPARE_GEOMETRY(p_detector_window[0])
	XMI_IF_COMPARE_GEOMETRY(p_detector_window[1])
	XMI_IF_COMPARE_GEOMETRY(p_detector_window[2])
	XMI_IF_COMPARE_GEOMETRY(n_detector_orientation[0])
	XMI_IF_COMPARE_GEOMETRY(n_detector_orientation[1])
	XMI_IF_COMPARE_GEOMETRY(n_detector_orientation[2])
	XMI_IF_COMPARE_GEOMETRY(area_detector)
	XMI_IF_COMPARE_GEOMETRY(acceptance_detector)
	XMI_IF_COMPARE_GEOMETRY(sigma_x)
	XMI_IF_COMPARE_GEOMETRY(sigma_xp)
	XMI_IF_COMPARE_GEOMETRY(sigma_y)
	XMI_IF_COMPARE_GEOMETRY(sigma_yp)
	XMI_IF_COMPARE_GEOMETRY(d_source_slit)
	XMI_IF_COMPARE_GEOMETRY(slit_size_x)
	XMI_IF_COMPARE_GEOMETRY(slit_size_y)

	after_geometry:

	//excitation
	if (A->excitation->n_discrete > 0 || B->excitation->n_discrete > 0) {
		if (A->excitation->n_discrete != B->excitation->n_discrete) {
			rv |= XMI_COMPARE_EXCITATION;
		}
		else {
			for (i = 0 ; i < A->excitation->n_discrete ; i++) {
				if (fabsl(A->excitation->discrete[i].energy-B->excitation->discrete[i].energy)/A->excitation->discrete[i].energy > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
				if (fabsl(A->excitation->discrete[i].horizontal_intensity-B->excitation->discrete[i].horizontal_intensity)/A->excitation->discrete[i].horizontal_intensity > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
				if (fabsl(A->excitation->discrete[i].vertical_intensity-B->excitation->discrete[i].vertical_intensity)/A->excitation->discrete[i].vertical_intensity > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
			}
		}
	}	

	if (A->excitation->n_continuous > 0 || B->excitation->n_continuous > 0) {
		if (A->excitation->n_continuous != B->excitation->n_continuous) {
			rv |= XMI_COMPARE_EXCITATION;
		}
		else {
			for (i = 0 ; i < A->excitation->n_continuous ; i++) {
				if (fabsl(A->excitation->continuous[i].energy-B->excitation->continuous[i].energy)/A->excitation->continuous[i].energy > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
				if (fabsl(A->excitation->continuous[i].horizontal_intensity-B->excitation->continuous[i].horizontal_intensity)/A->excitation->continuous[i].horizontal_intensity > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
				if (fabsl(A->excitation->continuous[i].vertical_intensity-B->excitation->continuous[i].vertical_intensity)/A->excitation->continuous[i].vertical_intensity > XMI_COMPARE_THRESHOLD) {
					rv |= XMI_COMPARE_EXCITATION;
					break;
				}
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

