/*
 *  Player.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */
#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <algorithm>
using std::max;
using std::min;

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include "Player.h"
#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Piece.h"


/*
 my move{
	find my best move
 }
 your move{
	assume your best move
	find my best move
 }
 my move{
	check your best vs your actuall
	continue on best
	or
	restart from actuall
 }
 */

Player::Player(){
	root = board(0x00000FFF, 0xFFF00000, 0);
	FFNN_setup(&evaluator);	
	return_board = board();
	
	my_color = WHITE;

	if (my_color == WHITE){
		return_board_val = 2.0;
		// start searching for my move
		// timedSearch()
	}else{
		return_board_val = -2.0;
		// pop off a thread to search for your move
	}
};

bool Player::newboard(board newb){
	root = newb;
	if(bool(return_board)){	//Check for cheaters
		moveGenerator nextmove(!my_color, return_board);
		bool valid = false;
		while (*nextmove) {
			if (*nextmove == newb){
				valid = true;
				break;
			}
			nextmove++;
		}
		return valid;
	}
	return true;
}

void Player::startTimmer(){ 
		deadline = clock::now() + move_time_limit;
	}

/* Moved to Player_Unix
board Player::get(){
	return return_board;
}
*/
void Player::serialSearch(){
	int max_depth = 6;
	int current_depth=1;
	cout << "Searching Depth: " << max_depth << endl;
	moveGenerator mg(my_color, root);
	return_board = mg.curBoard(); // assume first move is best until proven otherwise
	

	float new_val;
	try {//catch the timeout throw
		while (*mg){
			if(my_color == WHITE){
				new_val = bestBoard(my_color, *mg, current_depth, max_depth);
			}else{
				new_val = worstBoard(my_color, *mg, current_depth, max_depth);
			}
			
			if( (my_color == WHITE && new_val > return_board_val) ||
				(my_color == BLACK && new_val < return_board_val)){
				return_board_val = new_val;
				return_board = *mg;
			}
			mg++;
		}
		
	}
	catch (int e) {
		if (e != -1){
			cerr << "Unknown Error: " << e << endl;
		}
	}
}

float Player::bestBoard(piece_t p, board b, int current_depth, int max_depth){

		// I am terminal, so return my value
		if (terminal(b, current_depth, max_depth)){
			return FFNN_calculateOutputs(&evaluator, b);
		}
		
		
		//not a terminal board, so find the max of my children
		float val = -2.0;
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			val = max(val, worstBoard(!p, *nextmove, current_depth+1, max_depth));
			nextmove++;
		}
		
		return val;
	}

float Player::worstBoard(piece_t p, board b, int current_depth, int max_depth){
		
		// I am terminal, so return my value
		if (terminal(b, current_depth, max_depth)){
			return FFNN_calculateOutputs(&evaluator, b);
		}
		
		
		//not a terminal board, so find the max of my children
		float val = 2.0;
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			val = min(val, bestBoard(!p, b, current_depth, max_depth));
			nextmove++;
		}
		
		return val;
	}
									
bool Player::terminal(board b, int current_depth, int max_depth){
		if (current_depth >= max_depth)
			return true;
		
		if (false)// some kinda cache here
			return true;
		
		if(clock::now() >= deadline) //ran out of time
			throw -1;
	
		return false;
	}


/****************
 Threaded section
 ****************/


// does the actuall searching
void Player::parallelSearch(int lane_no, board my_root){
	
	cout_lock.lock();
	cout << "(" << lane_no << ") Setting board: " << endl;
	my_root.printBoard();
	cout_lock.unlock();

	board_lock.lock();
	return_board = my_root;
	board_lock.unlock();
return;


	try {
		int current_depth;
		int max_depth = 6;
		while(true){
			current_depth=1;
			cout_lock.lock();
			cout << "(" << lane_no << ") Max Depth: " << max_depth << endl;
			cout_lock.unlock();

			float new_val;
			moveGenerator mg(!my_color, my_root);
		
			while (*mg){
				if(my_color == WHITE){
					new_val = worstBoard(my_color, *mg, current_depth+1, max_depth);
				}else{
					new_val = bestBoard(my_color, *mg, current_depth+1, max_depth);
				}
				
				board_lock.lock();
				if ( (my_color == WHITE && new_val < return_board_val) ||
					(my_color == BLACK && new_val > return_board_val) ){
					return_board_val = new_val;
					cout_lock.lock();
					cout << "(" << lane_no << ") Better board found: "<< new_val << endl;
					mg->printBoard();
					cout_lock.unlock();
					return_board = my_root;
					return_board_val = new_val;
					
				}
				board_lock.unlock();
				mg++;
				
				boost::this_thread::interruption_point();
			}
			
			//max_depth++;
			break;
		}

	}
	catch (int e) {
		if (e != -1){
			cerr << "Unknown Error: " << e << endl;
		}
	}
}

// sets up threads
void Player::search(){
	
	/*
	 if(yourBest == newb){
	 // let thread continue
	 }else{
	 // kill thread and start new one
	 }*/
	
	clock::time_point real_start = clock::now();
	
	startTimmer();
	
	bool threaded = true;
	
	moveGenerator my_moves(my_color, root);
	board_lock.lock();
	return_board = *my_moves; // assume first move is best until proven otherwise
	board_lock.unlock();
	
	
	int lane_no=-1;
	while (*my_moves){
		lane_no++;
		
		cout_lock.lock();
		cout << "(Master) Creating thread: " << lane_no << endl;
		cout_lock.unlock();
		
		//move_lanes[lane_no] = *my_moves;
		try{
			branches.create_thread(boost::bind(&Player::parallelSearch, this, lane_no, *my_moves));
		}
		catch(...){
			threaded = false;
			break;
		}
		my_moves++;
		
		
//		if (lane_no >= 1)	// debug code
//			break;
	}
	
	if(!threaded){	// will try serial search
		cout << "(Master) Going serial" << endl;
		branches.interrupt_all();
		serialSearch();
		return;
	}
	
	cout_lock.lock();
	cout << "(Master) Branch Factor: " << lane_no+1 << endl;
	cout_lock.unlock();

	while(clock::now() < deadline){
		// wait
	}
	
	branches.interrupt_all();
	
	clock::time_point real_stop = clock::now();
	
//	typedef boost::chrono::milliseconds ms;
//	typedef boost::chrono::duration<double> sec;
	boost::chrono::duration<double>	lifespan = (real_stop - real_start);



	cout << "(Master) total time for search: " << lifespan  << endl;
}

// returns the current best board
board Player::get(){
	board_lock.lock();
	board return_copy = return_board;
	board_lock.unlock();
	return return_copy;
}
