/*
 *  Artemis.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 1/26/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <fstream>
#include <iostream>

#include <cstdlib>	// for std::size_t and drand48

using namespace std;

#include "moveGenerator.h"

int main(int argc, char * const argv[]){


	board NULLBOARD;
	board testBoard(0x00000F00, 0x00F00000, 0x00F00F00);
	
	moveGenerator mg(moveGenerator::WHITE, testBoard);
	
	cout << "mainBoard" << endl;
	testBoard.printBoard();
	
	
	while (mg.curBoard() != NULLBOARD){
		mg.curBoard().printBoard();
		mg.nextBoard();
	}
	
/*	
	mg = moveGenerator(moveGenerator::BLACK, testBoard);
	while (*mg != NULLBOARD){
		(*mg).printBoard();
		++mg;
	}
/* */
	
	return 0;
}
