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

#include <boost/chrono.hpp>
typedef boost::chrono::steady_clock	steady_clock;

#include "Board.h"
#include "Color.h"
#include "Player.h"
#include "moveGenerator.h"

#if defined(WIN32)
	#define test_file	"C:\\Users\\Greg\\Dropbox\\School\\Current\\CS_405\\Artemis.nn"
#elif defined(__APPLE__) || defined(unix)
	#define test_file	"/Users/greg/Dropbox/School/Current/CS_405/Artemis.nn"
#endif


int main(int argc, char * const argv[]){
		
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

	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	officialBoard.printBoard();

	Player MrBlack(WHITE);
	Player MrWhite(BLACK);
	
	color_t active_player = WHITE;
	size_t moves=0;
	
	
	steady_clock::time_point game_start = steady_clock::now();
	do{
		moves++;
		cout << "Turn: " << moves << endl;
		if(active_player == WHITE){
			if(!MrBlack.newboard(officialBoard)){
				std::cerr << "MrBlack calls cheater" << endl;
				cout << "No connection from" << endl;
				MrBlack.getmove().printBoard();
				exit(-1);
			}
			MrBlack.search();
			officialBoard = MrBlack.getmove();
		}else{
			if(!MrWhite.newboard(officialBoard)){
				std::cerr << "MrWhite calls cheater" << endl;
				cout << "No connection from" << endl;
				MrWhite.getmove().printBoard();
				exit(-1);
			}
			MrWhite.search();
			officialBoard = MrWhite.getmove();
		}
		
		officialBoard.printBoard();
		
		active_player = !active_player;
	}while (!officialBoard.winner());
	
	steady_clock::time_point game_end = steady_clock::now();
	boost::chrono::duration<double> d = game_end - game_start;
	
	cout << "Game time: " << d << endl;
	cout << (d)/moves << " per move" << endl;
	
	if(officialBoard.whitePawns){
		MrBlack.victory();
		MrWhite.defeat();
	}else{
		MrWhite.victory();
		MrBlack.defeat();
	}
	
	cout << "White" << endl;
	cout << MrBlack.toString() << endl;
	
	cout << "Black" << endl;
	cout << MrWhite.toString() << endl;
	
}

