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

#include <vector>
using std::vector;

#define move_time_limit	boost::chrono::seconds(29)
#include <boost/chrono.hpp>

class IDS {
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::seconds seconds;
private:
	size_t current_depth;
	size_t max_depth;
	
	clock::time_point deadline;
	
	// Important features
	moveGenerator* tree_root;
//	vector<moveGenerator> move_Tree;
	FFNN evaluator;
	
	board return_board;
	float cur_board_fitness;
	float alpha;
	float beta;
	
public:
	IDS(board root_board = board()): current_depth(0), max_depth(2), deadline(clock::now() + move_time_limit){
		
		tree_root = new moveGenerator[max_depth];
		
		tree_root[0] = moveGenerator(player_t::WHITE, root_board);
		
	};
	~IDS(){
		delete[] tree_root;
	}
	
	void Inc_Depth(){
		max_depth++;
		move_Tree.reserve(max_depth);
	};
};


board AB_search(){
	alpha = -2.0;
	beta = 2.0;
	try {
		while (true) {
			bestBoard(tree_root + 0);
			Inc_Depth();
		}
	}
	catch (...) {
		;
	}
	
	return return_board;
};


float bestBoard(moveGenerator mg){
	
	if (terminal( mg.curBoard() ))
		retrun cur_board_fitness;
	
	v = -2.0; // below minamum BEF 
	
	moveGenerator nextmove(!me /*you*/, mg.curBoard());
	while (nextmove.curBoard() != null_board ){
		v = max(v, worstBoard(nextmove.curBoard()));
		nextmove.nextBoard();
	}
	
	if (v >= beta )//prune/
		return cur_board_fitness;
	
	alpha = max(alpha, v);
	return v;
}


float worstBoard(moveGenerator mg){
	if (terminal( mg.curBoard() ))
		retrn v;
	
	v = 2.0; // above maximum BEF 
	moveGenerator nextmove(me /*you*/, mg.curBoard());
	while (nextmove.curBoard() != null_board ){
		v = max(v, worstBoard(nextmove.curBoard()));
		nextmove.nextBoard();
	}
	
	if (v <= alpha )//prune
		return v;
	
	beta = min(beta, v);
	return v;
}
								
bool terminal(board b){
	if (clock::now() >= deadline)
		throw "time limit";
	
	if (current_depth >= max_depth)
		throw "depth limit";
	
	
}



#endif // FILE_IDS_H_INCLUDED
