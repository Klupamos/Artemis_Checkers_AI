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
#include <string>
#include <vector>
#include <algorithm>

#include <boost/chrono.hpp>
typedef boost::chrono::steady_clock	steady_clock;

#include <boost/thread.hpp>

#include "Color.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Player.h"
#include "GUI.h"
#include "Training.h"

#if defined(WIN32)
	#define test_file	"C:\\Users\\Greg\\Dropbox\\School\\Current\\CS_405\\Artemis\\Training_nets\\0\\0"
#elif defined(__APPLE__) || defined(unix)
	#define test_file	"/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/0/0"
#endif

#define Save_Loc	std::string("/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/")

int main(int argc, char * const argv[]){
	
 
	// This is the final run code
	Training turny;
	if(!turny.load(Save_Loc)){
		cerr << "Error loading players!!" << endl;
		exit(0);
	}
	turny.run();
	exit(0);
/**/ 
	
	boost::chrono::milliseconds turn_time_limit(500);
	
	Player MrBlack;
	MrBlack.load(test_file);
	MrBlack.setColor(WHITE);
	MrBlack.setTimeLimit(turn_time_limit);
	
	Player MrWhite;
	MrWhite.load(test_file);
	MrWhite.setColor(BLACK);
	MrWhite.setTimeLimit(turn_time_limit);
	

/*  AB validation
	MrBlack.newboard(board(0x00000FFF, 0xFFF00000, 0));
	MrBlack.search(false);
	cout << "Parallel" << endl;
	cout << MrBlack.return_board_val << " / " << MrBlack.node_count << endl;
	MrBlack.getmove().printBoard();


	MrBlack.newboard(board(0x00000FFF, 0xFFF00000, 0));
	MrBlack.search(true);
	cout << "Serial" << endl;
	cout << MrBlack.return_board_val << " / " << MrBlack.node_count << endl;
	MrBlack.getmove().printBoard();

	exit(0);
/**/

	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	officialBoard.printBoard();
	
	color_t active_player = WHITE;
	size_t moves=0;
	
	steady_clock::time_point game_start = steady_clock::now();
	do{
		moves++;
		
		if(moves >= 80){break;}
		
		if(active_player == WHITE){
			if(!MrBlack.newboard(officialBoard)){
				std::cerr << "Whites calls cheater" << endl;
//				cout << "No connection from" << endl;
//				MrBlack.getmove().printBoard();
				exit(-1);
			}
//			if(moves != 1){
//				cout << "White thought it was" << endl;
//				MrBlack.getyourmove().printBoard();
//			}
			cout << "Whites move" << endl;
			MrBlack.search();
			officialBoard = MrBlack.getmove();
			MrBlack.thinkAhead();
			
		}else{
			if(!MrWhite.newboard(officialBoard)){
				std::cerr << "Blacks calls cheater" << endl;
//				cout << "No connection from" << endl;
//				MrWhite.getmove().printBoard();
				exit(-1);
			}
//			cout << "Black thought it was" << endl;
//			MrWhite.getyourmove().printBoard();
			cout << "Blacks move" << endl;
			MrWhite.search();
			officialBoard = MrWhite.getmove();
			MrWhite.thinkAhead();
		}
		
		officialBoard.printBoard();
		
		
		if(officialBoard == board()){	//active Player could not make a move
			if(active_player == WHITE){
				MrWhite.victory();
				MrBlack.defeat();
			}else{
				MrWhite.defeat();
				MrBlack.victory();
			}
			return 0;
		}
		
		
		
		active_player = !active_player;
	}while (!officialBoard.winner());
	
	steady_clock::time_point game_end = steady_clock::now();
	boost::chrono::duration<double> d = game_end - game_start;
	
	cout << d << "/" << moves << " = " << (d)/moves << " per move" << endl;
	
	if(officialBoard.whitePawns && !officialBoard.blackPawns){
		MrBlack.victory();
		MrWhite.defeat();
	}else if (officialBoard.blackPawns && !officialBoard.whitePawns){
		MrWhite.victory();
		MrBlack.defeat();
	}else{
		MrBlack.draw();
		MrWhite.draw();
	}

	
	cout << "White" << endl;
	cout << MrBlack << endl;
	
	cout << "Black" << endl;
	cout << MrWhite << endl;
/*	*/
	
	
	
	
	
	
	
	
}

