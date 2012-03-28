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

class Player { // can also be thought of as the player
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::seconds seconds;
	
private:
	piece_t my_color;
	FFNN evaluator;
	board root;
	
	
	clock::time_point deadline;
	boost::thread_group branches;
	boost::mutex cout_lock;// look at upgrade_locks and shared_locks	
	
	int node_count;
	
	board yourBest;
	
	boost::mutex alpha_lock;
	float global_alpha;

	boost::mutex beta_lock;
	float global_beta;

	boost::mutex board_lock;
	float return_board_val;
	board return_board;
	
	
public:
	Player(piece_t);

	void toString();

	bool newboard(board);
	void search();
	board getmove();
	

	void serialSearch();
private:
	void startTimmer();
	
	void parallelSearch(int, const board & );
	
	
	float bestBoard(piece_t, const board &, int, float, float);
	float worstBoard(piece_t, const board &, int, float, float);
	bool terminal(const board &, int);


friend std::istream & operator>>(std::istream &, Player &);
friend std::ostream & operator<<(std::ostream &, const Player &);
};




#endif // FILE_Player_H_INCLUDED
