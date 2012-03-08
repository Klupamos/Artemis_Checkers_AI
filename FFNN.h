/*
 *  PopulationEvolver.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_FFNN_H_INCLUDED
#define FILE_FFNN_H_INCLUDED


//define network layout
#define	def_total_layers	(4)
#define	def_layer_space		def_total_layers*sizeof(int)
#define	init_layers_macro(layer)	\
layer[0]=4;\
layer[1]=3;\
layer[2]=2;\
layer[3]=1;
#define def_input_layer		(4)
#define def_total_values	((3+1) + (2+2))
#define def_value_space		def_total_values*sizeof(float)
#define	def_total_weights	((4+0)*3 + (3+1)*2 + (2+2)*1)
#define def_weight_space	def_total_weights*sizeof(float)
#define	def_ptr_space		2*(def_total_layers*sizeof(float*))
#define	def_struct_space	def_value_space + def_weight_space + def_layer_space + def_ptr_space


/*	class FFNN
	Invariants:
		All pointers (unaligned_values, unaligned_weights, unaligned_weights[all], aligned_weights, layers, network_inputs, network_output)
		will be valid;
		1 <= king_value <= 2;
		0 < real_nodes <= padded_nodes;
		padded_nodes % 4 == 0
*/
struct FFNN {
	char* _main_memmory[def_struct_space + 15];
							
	float** aligned_values;
	float**	aligned_weights;	
	
	size_t layers_size;
	int* layers;

	float king_value;
	double variance;
};
	
float randNorm(double);

	void FFNN_setup(FFNN*);	// set up the network

	void FFNN_mutate(FFNN*, FFNN*); //this becomes a mutant of other	
	float FFNN_calculateOutputs(FFNN*, float*);			// outputs from the neural network

	void FFNN_printNetwork(FFNN*, float*, float);

	std::istream & operator>>(std::istream &, FFNN &);
	std::ostream & operator<<(std::ostream &, const FFNN &);


#endif	//#ifndef FILE_FFNN_H_INCLUDED
