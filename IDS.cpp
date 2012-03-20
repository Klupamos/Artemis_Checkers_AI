/*
 *  IDS.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */
#include <iostream>
using std::cout;
using std::endl;


#include <algorithm>
using std::max;
using std::min;


#include "IDS.h"
#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Person.h"



IDS::IDS(board root_board = board()): current_depth(0), max_depth(2), deadline(clock::now() + move_time_limit){
		root = root_board;
		FFNN_setup(&evaluator);
		return_board = board();
	};


board IDS::get(){
	return return_board;
}

void IDS::search(){
	
	moveGenerator mg(my_color, root);
	return_board = mg.curBoard(); // assume first move is best until proven otherwise
		
	float new_val = -2.0, old_val = -2.0;
	while (*mg){
		new_val = worstBoard(!my_color, *mg);
		
		if (new_val > old_val){
			old_val = new_val;
			return_board = *mg;
		}
		mg++;
	}
	cout << old_val << endl;
}

float IDS::bestBoard(player_t p, board b){
		current_depth++;
		
		// I am terminal, so return my value
		if (terminal( b )){
			current_depth--;
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		
		//not a terminal board, so find the max of my children
		float val = -2.0;
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			val = max(val, worstBoard(!p, *nextmove));
			nextmove++;
		}
		
		current_depth--;
		return val;
	}

float IDS::worstBoard(player_t p, board b){
		current_depth++;
		
		// I am terminal, so return my value
		if (terminal( b )){
			current_depth--;
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		
		//not a terminal board, so find the max of my children
		float val = 2.0;
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			val = min(val, bestBoard(!p, b));
			nextmove++;
		}
		
		current_depth--;
		return val;
	}
									
bool IDS::terminal(board b){
		if (current_depth >= max_depth)
			return true;
		
		if (false)// some kinda cache here
			return true;
		
		return false;
	}
