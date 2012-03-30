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

#include <string>

#include "mtrand.h"

#include "Board.h"
#include "Color.h"


//define network layout
#define	def_total_layers	(5)
#define	def_layer_space		def_total_layers*sizeof(int)
#define	init_layers_macro(layer)	\
layer[0]=36;\
layer[1]=56;\
layer[2]=24;\
layer[3]=8;\
layer[4]=1;
#define def_input_layer		(36)
#define def_total_values	((56+0) + (24+0) + (8+0))
#define def_value_space		def_total_values*sizeof(float)
#define	def_total_weights	((36+0)*56 + (56+0)*24 + (24+0)*8 + (8+0)*1)
#define def_weight_space	def_total_weights*sizeof(float)
#define	def_ptr_space		2*(def_total_layers*sizeof(float*))
#define	def_struct_space	def_value_space + def_weight_space + def_layer_space + def_ptr_space

/*	struct FFNN
	Invariants:
		All pointers (unaligned_values, unaligned_weights, unaligned_weights[all], aligned_weights, layers, network_inputs, network_output)
		will be valid;
		1 <= king_value <= 2;
*/
struct FFNN {
	char* _main_memmory[def_struct_space + 15];
							
	float** aligned_values;
	float**	aligned_weights;	
	
	int layers_size;
	int* layers;

	float king_value;
	double variance;

public:
	static MTRand_closed randGen;
};
	
	float randNorm(double);

	void FFNN_setup(FFNN*);	// set up the network

	void FFNN_mutate(FFNN*, FFNN*);						//first becomes a mutant of second	
	float FFNN_calculateOutputs(FFNN*, float*);			// outputs from the neural network
	float FFNN_calculateOutputs(FFNN*, const board &, color_t);

	void FFNN_printNetwork(FFNN*, float*, float);
	void FFNN_printNetwork(FFNN*, const board &, float);

	std::string FFNN_toString(const FFNN & network);

	std::istream & operator>>(std::istream &, FFNN &);
	std::ostream & operator<<(std::ostream &, const FFNN &);


#endif	//#ifndef FILE_FFNN_H_INCLUDED
