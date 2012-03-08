/*
 *  PopulationEvolver.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include "PopulationEvolver.h"

#include <iostream>
using std::cout; using std::endl;
#include <sstream>
using std::ostringstream;

#include <vector>
using std::vector;
#include <algorithm> //for std::sort
using std::sort;

#include <utility>
using std::make_pair;

#include <cstdlib>	// for drand48
using std::size_t;
// random_function = [-1.0, 1.0]
#if !defined(random_function)
#if defined(WIN32)
	#define random_function	2.0*rand()/(float)RAND_MAX - 1.0
#else
	#define random_function	2.0*drand48() - 1.0 
#endif
#endif



#define saveLocation "Artemis_save/"
#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost/filesystem.hpp>
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::directory_iterator;
using boost::filesystem::create_directories;
using boost::filesystem::last_write_time;
#include <boost/filesystem/fstream.hpp>
using boost::filesystem::ofstream;

// Generates or Read in neural networks
populationEvolver::populationEvolver(){
	
	
	path saveFiles = saveLocation;
	if(exists(saveFiles) && is_directory(saveFiles)){	// read in old population
		cout << "Reading old population" << endl;
		
		path newestFolder("/");
		
		directory_iterator end_iter;
		for(directory_iterator dir_iter(saveFiles) ; dir_iter != end_iter ; ++dir_iter){
			
			if ( is_directory(dir_iter->path()) ){
				cout << dir_iter->path().filename()  << endl;
			}
			
		}
        
		cout << "Now what? ... Exiting" << endl;
		exit(-1);
	}else{// create new population
		cout << "Creating new population" << endl;
		feedForwardNeuralNetwork* NN_ptr;
		int sizes[] = {4,3,2,1};
		
		
		while (population.size() < MAX_POPULATION){
			
			NN_ptr = new feedForwardNeuralNetwork(4, sizes);
			
			population.push_back(make_pair(NN_ptr, 0.0));
		}
	}
	
}


void populationEvolver::BigBang(){
	while (++generation){
		tournamnet();
		massExtenction();
		makeBabies();
		if (generation % 100 == 0){
			keepRecords();
			exit(-1);
		}
	}
}


void populationEvolver::tournamnet(){
	
};
/* Set them up in a tournament {W=1, D=0.5, L= -0.25} later moves to a {W=1, D=0, L=-2}
 Use Tournament Results as fitness
 */

// Ween out loosers {genetic algorithem}
bool pair_comp(const populationEvolver::weightedNet & a, const populationEvolver::weightedNet & b){
	return (a.second > b.second);
}
void populationEvolver::massExtenction(){
	sort(population.begin(), population.end(), pair_comp); // sorts population (biggest -> smallest)
}

void populationEvolver::makeBabies(){
	mutateNetwork();
};

void populationEvolver::mutateNetwork(){
	size_t kill_index;
	
	FFNN_mutate(population[kill_index].first, population[random_function].fitst);
}

void populationEvolver::keepRecords(){
	cout << "Saving population" << endl;
	path saveFiles = saveLocation;
	
	ostringstream fileDesc;
	fileDesc.precision(2);
	fileDesc << generation << "{" << population[0].second << "}";
	saveFiles /= fileDesc.str();
	
	if(exists(saveFiles) && is_directory(saveFiles)){ // make dir
		cout << "Error: " << saveFiles << " already exists!" << endl;
		cout << "\tOverwriting" << endl;
	}else
		create_directories(saveFiles);
	
	
	path saveFile;
	for(size_t index=0; index < population.size(); index++){
		saveFile = saveFiles;
		ostringstream indexstr;
		indexstr << index << ".dat";
		saveFile /= indexstr.str();
		
		ofstream outfile(saveFile, std::ios::out | std::ios::binary);
		outfile << *(population[index].first);
		outfile.close();
	}
	
	(population[0].first)->printNetwork();
	
}

void populationEvolver::readRecords(){

}