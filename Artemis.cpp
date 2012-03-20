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
#include <cstdlib>	// for std::size_t and drand48

using namespace std;

#include "Board.h"

#include "IDS.h"


int main(int argc, char * const argv[]){

	board testBoard(0x00000FFF, 0xFFF00000, 0);
	testBoard.printBoard();

	IDS test(testBoard);

	test.search();
	test.get().printBoard();
}

