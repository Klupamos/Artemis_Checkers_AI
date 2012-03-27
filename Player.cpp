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

//#define move_time_limit	boost::chrono::milliseconds(30000)
#define move_time_limit	boost::chrono::seconds(5)
#define search_start_height	4

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

Player::Player():node_count(0){
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
	startTimmer();
	
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

void Player::serialSearch(){
	return_board_val *= -1;
	
	global_alpha = -2.0;	// redundant from Player::search()
	global_beta = 2.0;		// debug/test code

	int current_height = search_start_height;
	moveGenerator default_move(my_color, root);
	return_board = *default_move; // assume first move is best until proven otherwise
	default_move.~moveGenerator();

	try {
		while (true){
			cout << "Searching Depth: " << current_height << endl;
			moveGenerator mg(my_color, root);
			
			cout_lock.lock();
			node_count++;
			cout_lock.unlock();
			
			while (*mg){
				if(my_color == WHITE){					
					global_alpha = max(global_alpha, worstBoard(!my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_alpha > return_board_val){
						return_board_val = global_alpha;
						return_board = *mg;
					}
				}else{
					global_beta = min(global_beta, bestBoard(!my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_beta < return_board_val){
						return_board_val = global_beta;
						return_board = *mg;
					}
				}
				
				mg++;
				
			}
			current_height++;
			break;//debug code
		}
	}
	catch (int e) {
		if(e!=-1){
			cerr << "Unknow error: " << e << endl;
			throw e;
		}
	}

	cout << "Counted nodes: " << node_count << endl;
}

float Player::bestBoard(piece_t p,const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
			return FFNN_calculateOutputs(&evaluator, b);
		}
		
		
		//not a terminal board, so find the max of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			alpha = max(alpha, worstBoard(!p, *nextmove, current_height-1, alpha, beta));
			
			if(alpha > beta)
				break;
			
			nextmove++;
		}
		
		return alpha;
	}

float Player::worstBoard(piece_t p, const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
			return FFNN_calculateOutputs(&evaluator, b);
		}
		
		
		//not a terminal board, so find the min of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			beta = min(beta, bestBoard(!p, *nextmove, current_height-1, alpha, beta));
			
			if(beta < alpha)
				break;
			
			nextmove++;
		}
		
		return beta;
	}
									
bool Player::terminal(const board & b, int current_height){
	
	cout_lock.lock();
	node_count++;
	cout_lock.unlock();
	
	
	if (current_height <= 0)
		return true;
	
	if (false)// some kinda cache here
		return true;
	
	if(clock::now() >= deadline){ //ran out of time
		cerr << "Throwing out of time error" << endl;
		throw -1;
	}

	return false;
}


/****************
 Threaded section
 ****************/


// does the actuall searching
void Player::parallelSearch(int lane_no, const board  & my_root){
	int current_height = search_start_height-1;
	
	while(true){
		cout_lock.lock();
		//cout << "(" << lane_no << ") Max Depth: " << current_height << endl;
		node_count++;
		cout_lock.unlock();

		float tmp_omega;
		moveGenerator mg(!my_color, my_root);
	
		while (*mg){
			try {
				
				if(my_color == WHITE){
					tmp_omega = max(global_alpha, worstBoard(my_color, *mg, current_height-1, global_alpha, global_beta));
					
					alpha_lock.lock();
					if(tmp_omega > global_alpha){
						global_alpha = tmp_omega; 
					}
					
					if(global_alpha > global_beta){
						break;
					}
					
					alpha_lock.unlock();


					board_lock.lock();
					if(tmp_omega > return_board_val){
						return_board_val = tmp_omega;
						return_board = my_root;
					}
					board_lock.unlock();
					
				}else{
					tmp_omega = min(global_beta, bestBoard(my_color, *mg, current_height-1, global_alpha, global_beta));
					
					beta_lock.lock();
					if(tmp_omega < global_beta){
						global_beta = tmp_omega;
					}
					
					if(global_beta < global_alpha){
						break;
					}
					
					
					beta_lock.unlock();

					board_lock.lock();
					if(tmp_omega < return_board_val){
						return_board_val = tmp_omega;
						return_board = my_root;
					}
					board_lock.unlock();

					
				}
			}
			catch (int e) {
				if (e == -1){
					boost::this_thread::interruption_point();
				}else{
					cerr << "Unknown error: " << e << endl;
					throw e;
				}
			}
			

			mg++;
			
			boost::this_thread::interruption_point();
		}
		
		//current_height++;
		break;// debug code
	}	
}

// sets up threads
void Player::search(){
	
	clock::time_point real_start = clock::now();

	/*
	 if(yourBest == newb){
	 // let thread continue
	 }else{
	 // kill thread and start new one
	 }*/
	
	
	
	global_alpha = -2.0;
	global_beta = 2.0;

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
			boost::this_thread::yield();
		}
		catch (boost::thread_resource_error tr){
			cerr << "Caught \'boost::thread_resource_error\'" << endl;
			threaded = false;
			break;
		}
		catch (std::bad_alloc ba) {
			cerr << "Caught \'std::bad_alloc\'" << endl;
			cerr << "\t" << "Thread: " << lane_no << endl;
			cerr << "\t" << "Board:  " << endl;
			my_moves->printBoard();
			
			threaded = false;
			break;
		}
		catch (...) {
			cerr << "Unknown error" << cout;
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
	boost::chrono::duration<double>	lifespan = (real_stop - real_start);
	cout << "(Master) total time for search: " << lifespan  << endl;
	cout << "Counted nodes: " << node_count << endl;
}

// returns the current best board
board Player::getmove(){
	board_lock.lock();
	board return_copy = return_board;
	board_lock.unlock();
	return return_copy;
}


std::istream & operator>>(std::istream & theStream, Player & p){
	theStream >> p.evaluator;
	return theStream;
}

std::ostream & operator<<(std::ostream & theStream, const Player & p){
	theStream << p.evaluator;
	return theStream;
}


void Player::toString(){
	FFNN_printNetwork(&evaluator, root, FFNN_calculateOutputs(&evaluator, root));
}