/*
 *  PopulationEvolver.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */


#include <iostream>
using std::cout;
using std::endl;

#include <cstdlib>		// for std::size_t and drand48
using std::size_t;

#include <ctime>		// for time()

#include <cmath>

// random_function = [-1.0, 1.0]
#define random_function	(2.0*FFNN::randGen() - 1.0)


#if defined(__APPLE__)
	#include "/Developer/usr/lib/clang/1.7/include/immintrin.h"
#else
	#include <intrin.h>	// for _mm128, _mm_setzero_ps, _mm_load_ps, _mm_add_ps, _mm_mul_ps
	// look at __m128 aligment allocator
#endif

#include "FFNN.h"
#include "Board.h"
#include "Person.h"

MTRand_closed FFNN::randGen((unsigned long)time(NULL));

/*	sigmoid()
	Pre: None
	Post: return == normalized value of x within (-1,1)
 */
inline float sigmoid(float x){
	return float(x / (0.25 + abs(x)));
}

inline float randNorm(double variance){
	double norm = 0.0;
	norm += random_function + random_function;
	norm += random_function + random_function;
	norm += random_function + random_function;
	norm += random_function + random_function;
	norm += random_function + random_function;
	norm += random_function + random_function;	// [-12, 12]
	norm *= variance/12.0;						// [-v, v]
	return (float)norm;
}

/*	Pre: argc > 0 && argv[argc-1] == 1
	Post: creates the following data allocation where x's are allocated only for padding perpourses

	example with passed values (argc = 4 , argv = {2,3,2,1}){
		layers_size = argc
		aligned_values = {0,1,x,x,  2,3,4,x,  5,6,x,x,  7,7,7,7}
		w1 = {0,1,x,x, 0,1,x,x, 0,1,x,x, ...}
		w2 = {2,3,4,x, 2,3,4,x, ...}
		w3 = {5,6,x,x, ...}
		layers = argv
		aligned_weights = {&w1, &w2, &w3, ...}
	}
 */
void FFNN_setup(FFNN *network){	
	
	network->layers_size = def_total_layers;
	
	network->king_value = (float)(random_function/2.0 + 1.5);	// [1, 2]
	network->variance = (float)(random_function/10.0);		// [-0.1, 0.1]
	
	long mem_offset = (((long)network->_main_memmory + 15) & (~0x0F));
	

	mem_offset += (def_value_space + def_weight_space);
	network->layers = (int*)(mem_offset);
	network->aligned_values = (float**)(mem_offset += def_layer_space);
	network->aligned_weights = (float**)(mem_offset += def_ptr_space/2);

	
	//reset to point to the weight_space
	long value_offset = (((long)network->_main_memmory + 15) & (~0x0F)); 
	long weight_offset = value_offset + def_value_space; 
	mem_offset = value_offset;
	

	// initialize new memmory
	init_layers_macro(network->layers);
	int  last_layer_size;
	for(size_t l=0; l<def_total_layers-1; l++){		
		// allocate space for values
		network->aligned_values[l] = (float*)(value_offset);
//		printf("Value layer %ld: %p\n", l, (void*)(value_offset - mem_offset));

		value_offset += (network->layers[l+1] + (3&(4-network->layers[l+1]))) * sizeof(float);
		
		// allocate space for weights
		network->aligned_weights[l] = (float*)(weight_offset);
//		printf("Weight layer %ld: %p\n", l, (void*)(weight_offset - mem_offset));
		last_layer_size = (network->layers[l] + (3&(4-network->layers[l])));
		weight_offset += (network->layers[l+1] * last_layer_size) * sizeof(float);

		// initialize random weights
		for (int n=0; n < network->layers[l+1]; n++){
			int w = 0;
			for (; w < network->layers[l]; w++){
				float r = (float)(random_function);
				*( network->aligned_weights[l] + n*last_layer_size  + w) = r;
			}
			for (; w < last_layer_size; w++){
				*( network->aligned_weights[l] + n*last_layer_size + w) = 0.0;
			}
		}
	}
}

/*	mutate
	Pre: child and parent are both initilized
	Post: child == a mutant of parent
 */
void FFNN_mutate(FFNN* child, FFNN* parent){
	int weight_index;
	float tmp;
	
	for(int l=0; l<def_total_layers-1; l++){									
		weight_index = 0;
		for (int node = 0; node < parent->layers[l+1]; node++) {
			for (int pre_node = 0; pre_node < parent->layers[l]; pre_node++, weight_index++) {
				tmp = parent->aligned_weights[l][weight_index] + randNorm(parent->variance);
				tmp = (tmp > 1.0 ? 1.0f : tmp);
				tmp = (tmp < -1.0 ? -1.0f : tmp);
				child->aligned_weights[l][weight_index] = tmp;
			}
			weight_index += (3&(4-weight_index));
		}
	}
	
	child->king_value = parent->king_value + randNorm(parent->variance);
	child->variance = parent->variance + randNorm(0.05);
}

/*	calculateOutputs()
	Pre: inputs.size >= layer[0]
	Post: 
 */
float FFNN_calculateOutputs(FFNN* network, float* inputs){
	
	int l0 = network->layers[0];
	int l1 = network->layers[1];
	
	float total_sum;
	
	__m128 sums;
	__m128 outputs;
	__m128 weights;
	 
	int weights_thing=0;
	// Input layer
	for (int node = 0; node<l1; node++){
		
		sums = _mm_setzero_ps();
		
		for (int pre_node = 0; pre_node<l0; pre_node+=4){
			// load weights into __m128
			outputs = _mm_loadu_ps(inputs + pre_node);
			weights = _mm_load_ps(network->aligned_weights[0] + weights_thing);
						
			// take thier dot product
			weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
			sums = _mm_add_ps(sums, weights);			// sum += weights
			weights_thing += 4;
		}
		sums = _mm_hadd_ps(sums, sums);
		sums = _mm_hadd_ps(sums, sums); 
		_mm_store_ss(&total_sum, sums);

		network->aligned_values[0][node] = sigmoid(total_sum);
	}
	
	
	// hidden layers
	for(size_t l=1; l<def_total_layers-2; l++){		
		weights_thing = 0;
		l0 = network->layers[l];
		l1 = network->layers[l+1];
		for(int node=0; node<l1; node++){
			sums = _mm_setzero_ps();
			
			for(int pre_node = 0; pre_node<l0; pre_node+=4){
				
				// load weights into __m128
				outputs = _mm_load_ps(network->aligned_values[l-1] + pre_node);
				weights = _mm_load_ps(network->aligned_weights[l] + weights_thing);

				// take thier dot product
				weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
				sums = _mm_add_ps(sums, weights);			// sum += weights
				weights_thing += 4;
				
			}
			sums = _mm_hadd_ps(sums, sums);
			sums = _mm_hadd_ps(sums, sums); 
			_mm_store_ss(&total_sum, sums);
			network->aligned_values[l][node] = sigmoid(total_sum);
		}
	}
	 
	
	// output layers
	weights_thing = 0;
	l0 = network->layers[def_total_layers-2];
	sums = _mm_setzero_ps();
	for(int pre_node = 0; pre_node<l0; pre_node+=4){
		 
		// load weights into __m128
		outputs = _mm_load_ps(network->aligned_values[def_total_layers-3] + pre_node);
		weights = _mm_load_ps(network->aligned_weights[def_total_layers-2] + weights_thing);
		 
		// take thier dot product
		weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
		sums = _mm_add_ps(sums, weights);			// sum += weights
		weights_thing += 4;
	}
	sums = _mm_hadd_ps(sums, sums);
	sums = _mm_hadd_ps(sums, sums); 
	_mm_store_ss(&total_sum, sums);
	return(sigmoid(total_sum));
}

float FFNN_calculateOutputs(FFNN* network, const board & bitboard, player_t p){
	
	float inputs[def_input_layer];
	
	inputs[0] = float(p);
	char white = 0;
	char black = 0;
	for(ULONG bit=1, index=4; index<def_input_layer; bit<<=1, index++){
		if(bitboard.whitePawns & bit){
			inputs[index] = 1.0;
			white++;
		}else if(bitboard.blackPawns & bit){
			inputs[index] = -1.0;
			black++;
		}else{
			inputs[index] = 0.0;
		}
		
		if(bitboard.kings & bit){
			inputs[index] *= network->king_value;
		}
		
	}
	inputs[1] = white;
	inputs[2] = black;
	
	
	int l0 = network->layers[0];
	int l1 = network->layers[1];
	
	float total_sum;
	
	__m128 sums;
	__m128 outputs;
	__m128 weights;
	
	int weights_thing=0;
	// Input layer
	for (int node = 0; node<l1; node++){
		
		sums = _mm_setzero_ps();
		
		for (int pre_node = 0; pre_node<l0; pre_node+=4){
			// load weights into __m128
			outputs = _mm_loadu_ps(inputs + pre_node);
			weights = _mm_load_ps(network->aligned_weights[0] + weights_thing);
			
			// take thier dot product
			weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
			sums = _mm_add_ps(sums, weights);			// sum += weights
			weights_thing += 4;
		}
		sums = _mm_hadd_ps(sums, sums);
		sums = _mm_hadd_ps(sums, sums); 
		_mm_store_ss(&total_sum, sums);
		
		network->aligned_values[0][node] = sigmoid(total_sum);
	}
	
	
	// hidden layers
	for(size_t l=1; l<def_total_layers-2; l++){		
		weights_thing = 0;
		l0 = network->layers[l];
		l1 = network->layers[l+1];
		for(int node=0; node<l1; node++){
			sums = _mm_setzero_ps();
			
			for(int pre_node = 0; pre_node<l0; pre_node+=4){
				
				// load weights into __m128
				outputs = _mm_load_ps(network->aligned_values[l-1] + pre_node);
				weights = _mm_load_ps(network->aligned_weights[l] + weights_thing);
				
				// take thier dot product
				weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
				sums = _mm_add_ps(sums, weights);			// sum += weights
				weights_thing += 4;
				
			}
			sums = _mm_hadd_ps(sums, sums);
			sums = _mm_hadd_ps(sums, sums); 
			_mm_store_ss(&total_sum, sums);
			network->aligned_values[l][node] = sigmoid(total_sum);
		}
	}
	
	
	// output layers
	weights_thing = 0;
	l0 = network->layers[def_total_layers-2];
	sums = _mm_setzero_ps();
	for(int pre_node = 0; pre_node<l0; pre_node+=4){
		
		// load weights into __m128
		outputs = _mm_load_ps(network->aligned_values[def_total_layers-3] + pre_node);
		weights = _mm_load_ps(network->aligned_weights[def_total_layers-2] + weights_thing);
		
		// take thier dot product
		weights = _mm_mul_ps(weights, outputs);		// weights *= outputs
		sums = _mm_add_ps(sums, weights);			// sum += weights
		weights_thing += 4;
	}
	sums = _mm_hadd_ps(sums, sums);
	sums = _mm_hadd_ps(sums, sums); 
	_mm_store_ss(&total_sum, sums);
	return(sigmoid(total_sum));
}

/*	printNetwork()
	Pre: None
	Post: None
 */
void FFNN_printNetwork(FFNN* network, float* inputs, float output){
	
	int l=0;
	
	
	cout << "King: " << network->king_value << endl;
	cout << "Var: " << network->variance << endl;
	
	
	printf("Input Layer\n");
	for(int n = 0; n<network->layers[l]; n++){
		printf("%ld, %d) %.2f\n", l, n, inputs[n]);
	}
	
	printf("Hidden Layers\n");
	for(l=1; l<network->layers_size-1; l++){									
		for(int n = 0; n<network->layers[l]; n++){							
			printf("%ld,%d) %.2f\n", l, n, network->aligned_values[l-1][n]);							
			printf("\tWeights\n");
			for(int pn = 0; pn<network->layers[l-1]; pn++){					
				printf("\t%.2f\t", network->aligned_weights[l-1][n*(network->layers[l-1] + (3&(4-network->layers[l-1]))) + pn]);
			}
			printf("\n");
		}
	}
	
	printf("Output Layer\n");
	printf("%ld, %d) %.2f\n",l, 0, output);
	printf("\tWeights\n");
	for(int pn = 0; pn<network->layers[l-1]; pn++){
		printf("\t%.2f\t", network->aligned_weights[l-1][pn]);
	}
	printf("\n");
	
}


void FFNN_printNetwork(FFNN* network, const board & bitboard, float output){
	
	float inputs[def_input_layer];
	
	inputs[0] = 3.0;
	char white = 0;
	char black = 0;
	for(ULONG bit=1, index=4; index<def_input_layer; bit<<=1, index++){
		if(bitboard.whitePawns & bit){
			inputs[index] = 1.0;
			white++;
		}else if(bitboard.blackPawns & bit){
			inputs[index] = -1.0;
			black++;
		}else{
			inputs[index] = 0.0;
		}
		
		if(bitboard.kings & bit){
			inputs[index] *= network->king_value;
		}
		
	}
	inputs[1] = white;
	inputs[2] = black;
	
	
	int l=0;
	
	cout << "King: " << network->king_value << endl;
	cout << "Var: " << network->variance << endl;
	
	
	printf("Input Layer\n");
	for(int n = 0; n<network->layers[l]; n++){
		printf("%ld, %d) %.2f\n", l, n, inputs[n]);
	}
	
	printf("Hidden Layers\n");
	for(l=1; l<network->layers_size-1; l++){									
		for(int n = 0; n<network->layers[l]; n++){							
			printf("%ld,%d) %.2f\n", l, n, network->aligned_values[l-1][n]);							
			printf("\tWeights\n");
			for(int pn = 0; pn<network->layers[l-1]; pn++){					
				printf("\t%.2f\t", network->aligned_weights[l-1][n*(network->layers[l-1] + (3&(4-network->layers[l-1]))) + pn]);
			}
			printf("\n");
		}
	}
	
	printf("Output Layer\n");
	printf("%ld, %d) %.2f\n",l, 0, output);
	printf("\tWeights\n");
	for(int pn = 0; pn<network->layers[l-1]; pn++){
		printf("\t%.2f\t", network->aligned_weights[l-1][pn]);
	}
	printf("\n");
	
}

/*	operator>>
	Pre: None
	Post: None
 */
std::istream & operator>>(std::istream & theStream, FFNN network){
	
	theStream.read((char*)&(network.layers_size), sizeof(size_t));
	if(network.layers_size != def_total_layers){
		std::exception e;
		throw(e);
	}
	
	theStream.read((char*)(network.layers), def_layer_space);
	theStream.read((char*)(network.aligned_weights[0]), def_weight_space);
	
	theStream.read((char*)&(network.king_value), sizeof(float));
	theStream.read((char*)&(network.variance), sizeof(double));
	
	return theStream;
}

/*	operator<<
	Pre: None
	Post: None
 */
std::ostream & operator<<(std::ostream & theStream, const FFNN &network){
	
	
	theStream.write((char*)&(network.layers_size), sizeof(size_t));
	theStream.write((char*)(network.layers), def_layer_space);
	theStream.write((char*)(network.aligned_weights[0]), def_weight_space);
	
	theStream.write((char*)&(network.king_value), sizeof(float));
	theStream.write((char*)&(network.variance), sizeof(double));
	

	return theStream;					
}




