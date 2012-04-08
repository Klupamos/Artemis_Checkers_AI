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
#include "Training.h"
#include "Client.h"

#if defined(WIN32)
	#define test_file	"C:\\Users\\Greg\\Dropbox\\School\\Current\\CS_405\\Artemis\\Training_nets\\0\\0"
#elif defined(__APPLE__) || defined(unix)
	#define test_file	"/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/0/0"
#endif

#define Save_Loc	std::string("/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/")

int main(int argc, char * const argv[]){

	
	// This is the client software to play in the class tournamnet
	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	
	
	Client user_client;
	
	if (!user_client.connect(argv[1], argv[2])){
		cout << "Could not conect to " << argv[1] << ":" << argv[2] << endl;
		return 0;
	}
	// enter GUI mode
	if(user_client.color() == SPEC){
		while (true) {
			user_client.recv(officialBoard);
			officialBoard.printBoard();
		}
	}
	
	
	Player user_Player;
	user_Player.setColor(user_client.color());
	user_Player.setTimeLimit(boost::chrono::milliseconds(500));
	
	bool first_move = false;
	bool ignore_next = false;
	moveGenerator mg(BLACK, officialBoard);
	
	
	if (user_client.color() == WHITE){		// server code says black goes first
		ignore_next = true;					// so ignore the servers first broadcast
		first_move = true;
	}
/********************/	
// Send move before game has started, overwriting the startgame the broadcast 
// Causes the server to stop listening to opponent
/********************/
/*
	if (user_client.color() == BLACK){
		ignore_next = true;
		first_move = false;
		goto jmp_start;
	}
/**/	
	while (true) {
		if(ignore_next){
			ignore_next = false;
			user_client.recv(officialBoard);
			std::cout << "Ignored" << std::endl;			
			
			if(officialBoard != board(0x00000FFF, 0xFFF00000, 0)){ // I ignored the wrong one
				goto jmp_start;
			}
		}
		
		user_client.recv(officialBoard);
		officialBoard.printBoard();

		jmp_start:
		if(first_move){
			first_move = false;
			while(*mg){
				if (*mg == officialBoard){goto jmp_start;}
				mg++;
			}
			user_client.send("invalid");
			exit(0);
		}
		
		if(!user_Player.newboard(officialBoard)){
			user_client.send("invalid");
			break;
		}
		user_Player.search();
		officialBoard = user_Player.getmove();
		user_Player.thinkAhead();
	
		
		user_client.send(officialBoard);
 
		officialBoard.printBoard();
		
		
		//if you hav no moves then I win
/*		moveGenerator mg(!user_client.color(), officialBoard);
		if (!*mg){
			user_client.send((user_client.color() == WHITE ? "white wins" : "black wins"));
			break;
		}
*/
	}
	
/**/
/*
	// This is the training run code
	Training turny;
	if(!turny.load(Save_Loc)){
		cerr << "Error loading players!!" << endl;
		exit(0);
	}
	turny.run();
	exit(0);
/**/
	
/*	
	boost::chrono::milliseconds turn_time_limit(2500);
	
	Player whiteplayer;
	whiteplayer.setColor(WHITE);
	whiteplayer.setTimeLimit(turn_time_limit);
	
	Player blackplayer;
	blackplayer.setColor(BLACK);
	blackplayer.setTimeLimit(turn_time_limit);
	

	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	officialBoard.printBoard();
	
	color_t active_player = BLACK;
	size_t moves=0;
	int bf;
	
	steady_clock::time_point game_start = steady_clock::now();
	do{
		moves++;
		
		if(moves >= 80){break;}
		
		if(active_player == WHITE){
			cout << "Whites move" << endl;
			bf = whiteplayer.newboard(officialBoard);
			if(!bf){
				std::cerr << "Whites calls cheater" << endl;
				cout << blackplayer.splat();
				exit(-1);
			}
			whiteplayer.search();
			officialBoard = whiteplayer.getmove();
			whiteplayer.thinkAhead();
		}else{
			cout << "Blacks move" << endl;
			bf = blackplayer.newboard(officialBoard);
			if(!bf){
				std::cerr << "Blacks calls cheater" << endl;
				cout << whiteplayer.splat();
				exit(-1);
			}			
			blackplayer.search();
			officialBoard = blackplayer.getmove();
			blackplayer.thinkAhead();
		}
		
		officialBoard.printBoard();
		
		
		if(officialBoard == board()){	//active Player could not make a move
			if(active_player == WHITE){
				blackplayer.victory();
				whiteplayer.defeat();
			}else{
				blackplayer.defeat();
				whiteplayer.victory();
			}
			break;
		}
		
		
		
		active_player = !active_player;
	}while (!officialBoard.winner());
	
	steady_clock::time_point game_end = steady_clock::now();
	boost::chrono::duration<double> d = game_end - game_start;
	
	cout << d << "/" << moves << " moves = " << (d)/moves << " per move" << endl;
	
	if(officialBoard.whitePawns && !officialBoard.blackPawns){
		whiteplayer.victory();
		blackplayer.defeat();
	}else if (officialBoard.blackPawns && !officialBoard.whitePawns){
		blackplayer.victory();
		whiteplayer.defeat();
	}else{
		whiteplayer.draw();
		blackplayer.draw();
	}

	
	cout << "White" << endl;
	cout << whiteplayer << endl;
	
	cout << "Black" << endl;
	cout << blackplayer << endl;
/*	*/
	
	
	
	
	
	
	
	
}

