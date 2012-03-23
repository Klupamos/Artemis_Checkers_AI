/*
 *  Player.h
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_PLAYER_H_INCLUDED
#define FILE_PLAYER_H_INCLUDED

#include <boost/chrono.hpp>
#include <boost/thread.hpp>


#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Piece.h"

//#define move_time_limit	boost::chrono::seconds(2)
#define move_time_limit	boost::chrono::milliseconds(2000)


class Player { // can also be thought of as the player
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::seconds seconds;
	
private:
	piece_t my_color;
	
	clock::time_point deadline;
	
	board root;

	boost::thread_group branches;
	
	board yourBest;
	
	FFNN evaluator;

	boost::mutex cout_lock;
	boost::mutex board_lock;	// look at upgrade_locks and shared_locks
	board return_board;
	float return_board_val;
	
public:
	Player();
	bool newboard(board);
	void search();
	void parallelSearch(int, board);
	void serialSearch();
	board get();

private:	
	float bestBoard(piece_t p, board b, int current_depth, int max_depth);
	float worstBoard(piece_t p, board b, int current_depth, int max_depth);
	bool terminal(board b, int current_depth, int max_depth);

	void startTimmer();
	
};




#endif // FILE_Player_H_INCLUDED
