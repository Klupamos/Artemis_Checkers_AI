/*
 *  Artemis.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include <iostream>
#include <fstream>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;

#include <cstdlib>	// for std::size_t and drand48

#include "Board.h"
#include "Player.h"

#if defined(WIN32)
	#define test_file	"C:\\Users\\Greg\\Dropbox\\School\\Current\\CS_405\\Artemis.nn"
#elif defined(__APPLE__) || defined(unix)
	#define test_file	"/Users/greg/Dropbox/School/Current/CS_405/Artemis.nn"
#endif

int main(int argc, char * const argv[]){
	
	Player Artemis;
	
/*
	std::ofstream ofile(test_file, std::ofstream::binary);
	ofile << Artemis;
	if (!ofile.is_open() || !ofile.good()){
		cout << "Write Error" << endl;
		return -1;
	}
	ofile.close();
	return 0;
/**/
	
	
	std::ifstream ifile(test_file, std::ifstream::binary);
	if (!ifile.is_open() || !ifile.good()){
		cout << "Read Error" << endl;
		return -1;
	}
//	ifile >> Artemis;
	ifile.close();


	board startBoard(0x00000FFF, 0xFFF00000, 0);
	startBoard.printBoard();

	bool valid;
	valid = Artemis.newboard(startBoard);
	if (!valid){
		cout << "Cheater" << endl;
		exit(-1);
	}
			
//	Artemis.search();
	Artemis.serialSearch();
	
	cout << "Choosen move: " << endl;
	Artemis.getmove().printBoard();
	
/*	
	valid = Artemis.newboard(board(0x00001EFF,0xFF780000,0));
	if (!valid){
		cout << "Cheater" << endl;
		board(0x00001EFF,0xFF780000,0).printBoard();
		exit (-1);
	}
	Artemis.timedSearch();
	Artemis.getmove().printBoard();
*/
}

