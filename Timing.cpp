/*
 *  Timing.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/2/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */


// Alternate main for timing functions


#include <fstream>
using std::ofstream;
using std::ios;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

#include <cstdlib>	// for drand48
#if !defined(srand48)
	#if defined(WIN32)
		#define srand48	srand
	#endif
#endif 

#include <ctime>	// for time(NULL)


#ifdef __APPLE__
	#define REPS	1000
#else
	#define REPS	100000
#endif


#include <boost/chrono.hpp>


#include "FFNN.h"



int main(int argc, char * const argv[]){
	
	srand48(3);
	try{
		
		FFNN human, mutant;
		
		FFNN_setup(&human);
		FFNN_setup(&mutant);
		
		float inputs[def_input_layer];
		for (int i=0; i<def_input_layer; i++)
			inputs[i] = 1.0;
		
		float output;
		
		boost::chrono::system_clock::time_point start;
		boost::chrono::duration<double> dur1;
		boost::chrono::duration<double> dur2;
		boost::chrono::duration<double> dur3;
		boost::chrono::duration<double> avg;
		
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_setup(&human);
		dur1 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_setup(&human);
		dur2 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_setup(&human);
		dur3 = boost::chrono::system_clock::now() - start;
		
		avg = (dur1 + dur2 + dur3) / 3.0;
		cout << "3 .setup() trials averaging " << avg.count() << " seconds. " << REPS/avg.count() << " function calls per second" << "\n";
		cout << "\tTrial 1: " << dur1.count() << " seconds\n";
		cout << "\tTrial 2: " << dur2.count() << " seconds\n";
		cout << "\tTrial 3: " << dur3.count() << " seconds\n";
		cout << flush;
		
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			output = FFNN_calculateOutputs(&human, inputs);
		dur1 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			output = FFNN_calculateOutputs(&human, inputs);
		dur2 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			output = FFNN_calculateOutputs(&human, inputs);
		dur3 = boost::chrono::system_clock::now() - start;
		
		avg = (dur1 + dur2 + dur3) / 3.0;
		cout << "3 .calculateOutputs() trials averaging " << avg.count() << " seconds. " << REPS/avg.count() << " function calls per second" << "\n";
		cout << "\tTrial 1: " << dur1.count() << " seconds\n";
		cout << "\tTrial 2: " << dur2.count() << " seconds\n";
		cout << "\tTrial 3: " << dur3.count() << " seconds\n";
		cout << flush;
		
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_mutate(&human, &mutant);
		dur1 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_mutate(&human, &mutant);
		dur2 = boost::chrono::system_clock::now() - start;
		
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			FFNN_mutate(&human, &mutant);
		dur3 = boost::chrono::system_clock::now() - start;
		
		avg = (dur1 + dur2 + dur3) / 3.0;
		cout << "3 .mutate() trials averaging " << avg.count() << " seconds. " << REPS/avg.count() << " function calls per second" << "\n";
		cout << "\tTrial 1: " << dur1.count() << " seconds\n";
		cout << "\tTrial 2: " << dur2.count() << " seconds\n";
		cout << "\tTrial 3: " << dur3.count() << " seconds\n";
		cout << flush;
		
		
		ofstream file;
		file.open("tmp.bin", ios::out | ios::binary);
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			file << human << endl;
		dur1 = boost::chrono::system_clock::now() - start;
		file.close();
		
		file.open("tmp.bin", ios::out | ios::binary);
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			file << human << endl;
		dur2 = boost::chrono::system_clock::now() - start;
		file.close();
		
		file.open("tmp.bin", ios::out | ios::binary);
		start = boost::chrono::system_clock::now();
		for(int i=0; i < REPS; i++)
			file << human << endl;
		dur3 = boost::chrono::system_clock::now() - start;
		file.close();
		
		avg = (dur1 + dur2 + dur3) / 3.0;
		cout << "3 .output() trials averaging " << avg.count() << " seconds. " << REPS/avg.count() << " function calls per second" << "\n";
		cout << "\tTrial 1: " << dur1.count() << " seconds\n";
		cout << "\tTrial 2: " << dur2.count() << " seconds\n";
		cout << "\tTrial 3: " << dur3.count() << " seconds\n";
		cout << flush;
		
		
	}
	catch(...){
		cout << "Something Threw" << endl;
		exit(-1);
	}
	
	return 0;
}
