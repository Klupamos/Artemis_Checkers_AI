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

int main(int argc, char * const argv[]){


	board testBoard(0x00000FFF, 0xFFF00000, 0);
	testBoard.printBoard();

	bool valid;
	Player Kallor;
	valid = Kallor.newboard(testBoard);
	if (!valid){
		cout << "Cheater" << endl;
		exit(-1);
	}
			
	Kallor.search();
	Kallor.get().printBoard();
	
/*	
	valid = Kallor.newboard(board(0x00001EFF,0xFF780000,0));
	if (!valid){
		cout << "Cheater" << endl;
		board(0x00001EFF,0xFF780000,0).printBoard();
		exit (-1);
	}
	Kallor.timedSearch();
	Kallor.get().printBoard();
*/
}

