/*
 *  IDS.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */


#include "IDS.h"


// rewrite min and max to avoid branching
float min(float a, float b){
	return (a<b)*a + (b<a)*b;
}
float max(float a, float b){
	return (a<b)*b + (b<a)*a;
}

board IDS(board root, FFNN evaluator, size_t depth, boost::chrono::steady_clock::time_point deadline){
	board return_board;
	player_t me = player_t::WHITE;
	
	moveGenerator nth_move(me, root);
	return_board = nth_move.curBoard();
	
	try{
		return_board = bestBoard();
	}catch (...) {
		
	}
	
	return return_board;
}

bestBoard(board, size_t depth, float a, float b){
	if (terminal(board))
		retrn v;
	
	v = -inf; // below minamum BEF 
	for each move m from board {
		v = max(v, worstBoard(board(m), depth, a, b));
	}s
	
	if (v >= b )//prune/
		return v;
	
	a = max(a, v);
	return v;
}

worstBoard(board, size_t depth, float a, float b){
	if (terminal(board))
		retrn v;
	
	v = inf; // above maximum BEF 
	for each move m from board {
		v = min(v, bestBoard(board(m), depth, a, b));
	}
	
	if (v <= a )//prune
		return v;
	
	b = min(b, v);
	return v;
}

typedef boost::chrono::steady_clock::time_point	steady_time_point;
bool terminal(board b, size_t d, const steady_time_point & start){
	
	if (boost::chrono::steady_clock::now() >= start + boost::chrono::seconds(move_time_limit) )// ran out of time
		return true;
	
	if (depth >= max_depth)//	- at max depth
		return true;
	//	- end game database search
	//	- cache
	return false;
}

ab_Search(board, moveGenerator::player_t, p, size_t depth){
	float v = bestBoard(board, depth, -2.0, 2.0);
	return v;
}

