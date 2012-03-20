/*
 *  IDS.h
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_IDS_H_INCLUDED
#define FILE_IDS_H_INCLUDED

#include <iostream>	//for error reporting

#include <boost/chrono.hpp>
#define move_time_limit	boost::chrono::seconds(29)

#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Person.h"

class IDS { // can also be thought of as the player
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::seconds seconds;

private:
	player_t my_color;
	
	size_t current_depth;
	size_t max_depth;
	
	clock::time_point deadline;
	
	board root;
	
	FFNN evaluator;
	
	board return_board;

	
public:
	IDS(board root_board);
	

	void search();
	board get();

	float bestBoard(player_t, board);
	float worstBoard(player_t, board);

	bool terminal(board);

	inline void startTimmer(){ 
		deadline = clock::now() + move_time_limit;
	}
};




#endif // FILE_IDS_H_INCLUDED
