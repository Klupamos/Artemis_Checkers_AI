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
#include "Piece.h"
#include "Player.h"
#include "moveGenerator.h"

#if defined(WIN32)
	#define test_file	"C:\\Users\\Greg\\Dropbox\\School\\Current\\CS_405\\Artemis.nn"
#elif defined(__APPLE__) || defined(unix)
	#define test_file	"/Users/greg/Dropbox/School/Current/CS_405/Artemis.nn"
#endif

int main(int argc, char * const argv[]){
	
/*		
	board test(1<<19, 0xF8800000, 0);
	test.printBoard();
	
	moveGenerator x(WHITE, test);
*/	


	
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
	
/*	std::ifstream ifile(test_file, std::ifstream::binary);
	if (!ifile.is_open() || !ifile.good()){
		cout << "Read Error" << endl;
		return -1;
	}
	ifile >> Artemis;
	ifile.close();
*/

	board startBoard(0x00000FFF, 0xFFF00000, 0);
	startBoard.printBoard();

	Player truth(WHITE);
	Player justice(BLACK);
	
	piece_t active_player = WHITE;
	size_t moves=0;
	
	while (!startBoard.winner()) {
		cout << "Turn: " << moves << endl;
		if(active_player == WHITE){
			if(!truth.newboard(startBoard)){
				std::cerr << active_player << " calls cheater" << endl;
				exit(-1);
			}
			truth.search();
			startBoard = truth.getmove();
		}else{
			if(!justice.newboard(startBoard)){
				std::cerr << active_player << " calls cheater" << endl;
				exit(-1);
			}
			justice.search();
			startBoard = justice.getmove();
		}
		
		startBoard.printBoard();
		moves++;
		active_player = !active_player;
	}
}

