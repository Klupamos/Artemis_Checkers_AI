/*
 *  GUI.h
 *  Artemis
 *
 *  Created by Gregory Klupar on 3/29/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_GUI_H_INCLUDED
#define FILE_GUI_H_INCLUDED 

#include <vector>
using std::vector;

#include <boost/chrono.hpp>

#include "Color.h"
#include "Board.h"
#include "Player.h"

struct interface{

	typedef boost::chrono::steady_clock	clock;

	Player MrBlack;
	Player MrWhite;
	
	board officialBoard;

	vector<board> inspection_board;	//sequence of moves
	int inspection_num; // bound between [0, turn_num]

	int turn_num;
	clock::time_point turn_start;

	bool game_paused;
	bool menu_displayed;
	
	interface();
};


void drawBoard();
void drawMenu();

// glut main function
void GUI_inti(int, char**);

// glut functions
void renderDisplay();
void keboardInput(unsigned char key, int x, int y);
void idle();
void reshape();
void mouse();

#endif //FILE_GUI_H_INCLUDED