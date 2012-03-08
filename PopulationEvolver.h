/*
 *  PopulationEvolver.h
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_POPULATION_EVOLVER_H_INCLUDED
#define FILE_POPULATION_EVOLVER_H_INCLUDED

#include <utility>	//for Pair
using std::pair;
#include <vector>	//for Pair
using std::vector;

#include "FFNN.h"

class populationEvolver{
public:
	typedef pair<feedForwardNeuralNetwork*, float> weightedNet;

	enum {
		MAX_POPULATION = 10,
		PERCENT_SURVAVIAL = 50	// should be 1 / generation
	};

private:
	size_t generation;
	size_t best_network_index;
	float  best_network_fitness;
	vector<weightedNet> population;
	
public:
	// Generates or Read in neural networks
	populationEvolver();
		
public:
	//Main looping function
	void BigBang();

private:
	void tournamnet();
	/* Set them up in a tournament {W=1, D=0.5, L= -0.25} later moves to a {W=1, D=0, L=-2}
	 Use Tournament Results as fitness
	 */
	
	// Ween out loosers {genetic algorithem}
	void massExtenction();
	
	void makeBabies();
	
	void mutateNetwork();

	void keepRecords();
	
	void readRecords();
		
};	// end class populationEvolver

#endif // #ifndef FILE_POPULATION_EVOLVER_H_INCLUDED