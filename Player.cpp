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

#include <sstream>
#include <string>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

#include "Player.h"
#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Color.h"

//#define move_time_limit	boost::chrono::milliseconds(30000)
#define move_time_limit	boost::chrono::seconds(2)
#define search_start_height 6

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

Player::Player(color_t color = WHITE):turny_points(0), games_played(0){
	root = board(0x00000FFF, 0xFFF00000, 0);

	FFNN_setup(&evaluator);	

	return_board = board();
	
	my_color = color;

	if (my_color == WHITE){
		return_board_val = -2.0;
		// start searching for my move
		// timedSearch()
	}else{
		return_board_val = 2.0;
		// pop off a thread to search for your move
	}
};

bool Player::newboard(board newb){
	startTimmer();
	
	root = newb;
	if(bool(return_board)){	//Check for cheaters
		moveGenerator nextmove(!my_color, return_board);
		while (*nextmove) {
			if (*nextmove == newb){
				return true;
			}
			nextmove++;
		}
		return false;
	}
	return true;
}

void Player::startTimmer(){ 
	deadline = clock::now() + move_time_limit;
}

void Player::serialSearch(){	
	int current_height = search_start_height;
	
	try {
		global_alpha = -2.0;	// redundant
		global_beta = 2.0;		//  from search
		return_board_val = (my_color == WHITE ? -2.0 : 2.0 ); //each itteration starts with the same value
		while (true){
			
			moveGenerator mg(my_color, root);
			
//			cout_lock.lock();
//			node_count++;		//debug
//			cout_lock.unlock();
			
			while (*mg){
				if(my_color == WHITE){
					global_alpha = max(global_alpha, worstBoard(!my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_alpha > return_board_val){
//						cout << "New board: " << global_alpha << " vs " << return_board_val << endl; //debug
						return_board_val = global_alpha;
						return_board = *mg;
					}
				}else{
					global_beta = min(global_beta, bestBoard(!my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_beta < return_board_val){
//						cout << "New board: " << global_beta << " vs " << return_board_val << endl; //debug
						return_board_val = global_beta;
						return_board = *mg;
					}
				}
				
				mg++;
				
			}
//			cout << "Finished Ply: " << current_height << endl;//debug
			current_height++;
		}
	}
	catch (int e) {
		if(e!=-1){
			cerr << "Unknow error: " << e << endl;
			throw e;
		}
	}

//	cout << "Counted nodes: " << node_count << endl;
}

float Player::bestBoard(color_t p,const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
			return FFNN_calculateOutputs(&evaluator, b, p);
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

float Player::worstBoard(color_t p, const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
			return FFNN_calculateOutputs(&evaluator, b, p);
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
	node_count++;		//debug
	cout_lock.unlock();
	
	
	if (current_height <= 0)
		return true;
	
	if (false)// some kinda cache here
		return true;
	
	if(clock::now() >= deadline){ //ran out of time
//		cout_lock.lock();
//		cerr << "Throwing out of time error" << endl; //debug
//		cout_lock.unlock();
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
	float tmp_omega;
	bool obsolete;
	
	while(true){
		
//		cout_lock.lock();
//		node_count++;		//debug
//		cout_lock.unlock();
		
		moveGenerator mg(!my_color, my_root);
		while (*mg){
			try {
				obsolete = false;
				if(my_color == WHITE){
					tmp_omega = max(global_alpha, worstBoard(my_color, *mg, current_height-1, global_alpha, global_beta));
					boost::this_thread::interruption_point();// prevent board change after timeout
					
					alpha_lock.lock();
					if(tmp_omega > global_alpha){
						global_alpha = tmp_omega; 
					}
					obsolete = (global_alpha > global_beta);
					alpha_lock.unlock();
					
					if(obsolete){
						break;//debug??
					}


					board_lock.lock();
					if(tmp_omega > return_board_val){
						//cout << "WHITE New board: " << tmp_omega << " vs " << return_board_val << endl; //debug
						return_board_val = tmp_omega;
						return_board = my_root;
					}
					board_lock.unlock();
					
				}else{
					tmp_omega = min(global_beta, bestBoard(my_color, *mg, current_height-1, global_alpha, global_beta));
					boost::this_thread::interruption_point();
					
					beta_lock.lock();
					if(tmp_omega < global_beta){
						global_beta = tmp_omega;
					}
					obsolete = (global_alpha > global_beta);
					beta_lock.unlock();
					
					if(obsolete){
						break;//debug??
					}

					board_lock.lock();
					if(tmp_omega < return_board_val){
						//cout << "BLACK New board: " << tmp_omega << " vs " << return_board_val << endl; //debug
						return_board_val = tmp_omega;
						return_board = my_root;
					}
					board_lock.unlock();
				}
			}
			catch (int e) {
				if (e == -1){
/*					cout_lock.lock();
					cerr << "Interupting Thread" << endl; //debug
					cout_lock.unlock();
*/					boost::this_thread::interruption_point();
				}else{
					cerr << lane_no << " Unknown error: " << e << endl;
					throw e;
				}
			}
			mg++;
			
			boost::this_thread::interruption_point();
		}
/*		
		cout_lock.lock();
		cout << "(" << lane_no << ") Finished Ply: " << current_height+1 << endl;//debug
		cout_lock.unlock();
*/
		current_height++;
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
	bool threaded = true;
	
	moveGenerator my_moves(my_color, root);
	board_lock.lock();
	global_alpha = -2.0; // before threads
	global_beta = 2.0;
	return_board_val = (my_color == WHITE ? -2.0 : 2.0); //each itteration starts with the same values
	return_board = *my_moves; // assume first move is best until proven otherwise
	board_lock.unlock();
	
	

	int lane_no=-1;
	while (*my_moves){
		lane_no++;
		
//		cout_lock.lock();
//		cout << "(Master) Creating thread: " << lane_no << endl;//debug
//		cout_lock.unlock();
		
		try{
			branches.create_thread(boost::bind(&Player::parallelSearch, this, lane_no, *my_moves));
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
	}
	
	if (lane_no == 0 && threaded){//only one move available
		goto only_move_cut_foo;
	}
	
	if(!threaded){	// will try serial search
		cout << "(Master) Going serial" << endl;
		branches.interrupt_all();
		serialSearch();
		return;
	}
	
	cout_lock.lock();
	cout << "(Master) Branch Factor: " << lane_no+1 << endl;
	bf.push_back(lane_no+1);
	cout_lock.unlock();

	while(clock::now() < deadline){
		// wait
	}
	
only_move_cut_foo:
	branches.interrupt_all();
	
//	clock::time_point real_stop = clock::now();
//	boost::chrono::duration<double>	lifespan = (real_stop - real_start);
//	cout_lock.lock(); 	
//	cout << "(Master) total time for search: " << lifespan  << endl;//debug
//	cout << "Counted nodes: " << node_count << endl;
//	cout_lock.unlock();
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

std::string Player::toString(){
	std::stringstream report(std::stringstream::out);
	report << "Points earned: " << turny_points << "\n";
	report << "Games player:  " << games_played << "\n";
	report << FFNN_toString(evaluator) << endl;
	return report.str();
}