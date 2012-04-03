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

#include <fstream>

#include <algorithm>
using std::max;
using std::min;

#include <sstream>
#include <string>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "Player.h"
#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Color.h"

//#define move_time_limit	boost::chrono::milliseconds(30000)
#define move_time_limit	boost::chrono::seconds(4)
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

Player::Player():age(0), turny_points(0){
	root = board(0x00000FFF, 0xFFF00000, 0);
	return_board = board();
	
	FFNN_setup(&evaluator);
}

void Player::setColor(color_t color){
	my_color = color;
}

bool Player::newboard(const board & newb){
	startTimmer();
	node_count = 0;
	
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
	move_deadline = clock::now() + move_time_limit;
}

void Player::serialSearch(){	
	int current_height = search_start_height;
	
	try {
		global_alpha = -2.0;
		global_beta = 2.0;
		return_board_val = (float)(my_color == WHITE ? -2.0 : 2.0 ); //each itteration starts with the same value

		node_count = 0;
		while (true){
			
			moveGenerator mg(my_color, root);
			
			cout_lock.lock();
			node_count++;		//debug for root node
			cout_lock.unlock();
			
			while (*mg){
				
				if(my_color == WHITE){

					global_alpha = max(global_alpha, worstBoard(0, !my_color, *mg, current_height-1, global_alpha, global_beta));
//					cout << "New board: " << global_alpha << " cmp " << return_board_val << endl; //debug

					if(global_alpha > return_board_val){
//						cout << "New board: " << global_alpha << " vs " << return_board_val << endl; //debug
						return_board_val = global_alpha;
						return_board = *mg;
						yourBest = yourmoves[0];	// save What I think your move will be
					}
				}else{
					global_beta = min(global_beta, bestBoard(0, !my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_beta < return_board_val){
//						cout << "New board: " << global_beta << " vs " << return_board_val << endl; //debug
						return_board_val = global_beta;
						return_board = *mg;

						yourBest = yourmoves[0];	// save What I think your move will be
					}
				}
				
				mg++;
				
			}
			cout << "Finished Ply: " << current_height << endl;//debug
			break;//debug oncethrough
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

float Player::bestBoard(char lane, color_t p,const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
//			cout << "B: " << FFNN_calculateOutputs(&evaluator, b, p) << endl;
//			b.printBoard();//debug
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		board tmp;
		float omega;
		float t;
		//not a terminal board, so find the max of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
//			cout << "B: " << current_height << " - " << alpha << "/" << beta << endl;
			t = worstBoard(-1, !p, *nextmove, current_height-1, alpha, beta);
//			cout << "B: " << current_height << " - " << t << endl;
			omega = max(alpha, t);
			if(omega > alpha){
				alpha = omega;
				tmp = *nextmove;	// this is your current best move
			}

			if(alpha > beta)
				break;
			nextmove++;
		}

		if(lane >= 0){	// save opponents best board
			yourmoves[lane] = tmp;
		}
		return alpha;
	}

float Player::worstBoard(char lane, color_t p, const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
//			cout << "W: " << FFNN_calculateOutputs(&evaluator, b, p) << endl;
//			b.printBoard();//debug
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		board tmp;
		float t;
		float omega;
		//not a terminal board, so find the min of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
//			cout << "W: " << current_height << " - " << alpha << "/" << beta << endl;
			t = bestBoard(-1, !p, *nextmove, current_height-1, alpha, beta);
//			cout << "W: " << current_height << " - " << t << endl;
			omega = min(beta, t);
			if(omega < beta){
				beta = omega;
				tmp = *nextmove;
			}

			if(beta < alpha)
				break;

			nextmove++;
		}

		if(lane >= 0){	// save opponents best board
			yourmoves[lane] = tmp;
		}
		
		return beta;
	}
									
bool Player::terminal(const board & b, int current_height){
	
	cout_lock.lock();
	node_count++;		//debug
	cout_lock.unlock();
	
	if(clock::now() >= move_deadline){ //ran out of time
		cout_lock.lock();
		cerr << "Throwing out of time error" << endl; //debug
		cout_lock.unlock();
		throw -1;
	}

	if (current_height <= 0)
		return true;
	
	if (b.winner())
		return true;

	return false;
}


/****************
 Threaded section
 ****************/


// does the actuall searching
void Player::parallelSearch(int lane_no, const board & my_root){

	int current_height = search_start_height-1;
	float tmp_omega;
	float local_alpha = global_alpha;	
	float local_beta = global_beta;		// create local copies of global variables
	bool obsolete;

	try {
		while(true){
		
			
			moveGenerator mg(!my_color, my_root);
			while (*mg){
				
				obsolete = false;
				if(!my_color == WHITE){
					tmp_omega = max(local_alpha, worstBoard(-1, my_color, *mg, current_height-1, local_alpha, local_beta));
					
					if(tmp_omega > local_alpha){
						local_alpha = tmp_omega; 
					}

					obsolete = (local_alpha > local_beta);
					if(obsolete)	// this is alpha-beta pruning
						break;
				}else{
					tmp_omega = min(local_beta, bestBoard(-1, my_color, *mg, current_height-1, local_alpha, local_beta));
				
					if(tmp_omega < local_beta){
						local_beta = tmp_omega;
						yourmoves[lane_no] = *mg;
					}

					obsolete = (local_alpha > local_beta);
					if(obsolete)	// this is alpha-beta pruning
						break;
				}
				mg++;
			}

			// This is what would be in the top most node
			if(my_color == WHITE){ // Max node
			
				tmp_omega = max(local_alpha, local_beta);
			
				alpha_lock.lock();
				if (tmp_omega > local_alpha){local_alpha = tmp_omega;}
				alpha_lock.unlock();

	//			cout_lock.lock();
	//			cout << "(" << lane_no << ")New board: " << global_alpha << " vs " << return_board_val << endl; //debug
	//			cout_lock.unlock();

				board_lock.lock();
				if(tmp_omega > return_board_val){
					return_board_val = tmp_omega;
					return_board = my_root;

					//where is yourmoves set??
					yourBest = yourmoves[lane_no];	// save What I think your move will be
				}
				board_lock.unlock();
			}else{ // Min node
				tmp_omega = min(local_beta, local_alpha);
			
				beta_lock.lock();
				if (tmp_omega < local_beta){local_beta = tmp_omega;}
				beta_lock.unlock();

				board_lock.lock();
				if(tmp_omega < return_board_val){
					return_board_val = tmp_omega;
					return_board = my_root;

					yourBest = yourmoves[lane_no];	// save What I think your move will be
				}
				board_lock.unlock();
			}

			cout_lock.lock();
			cout << "(" << lane_no << ") Finished Ply: " << current_height+1 << " / " << return_board_val << endl;//debug
			cout_lock.unlock();
			
			break;//debug oncethrough
			current_height++;
		}	
	}
	catch (int e) {
		if (e == -1){
//			cout_lock.lock();
//			cerr << "Thread Timming Interupt" << endl; //debug
//			cout_lock.unlock();
			return;
		}else{
			cerr << lane_no << " Unknown error: " << e << endl;
			throw e;
		}
	}
}

// sets up threads
// All threads created herein will eventually run out of time and close thmeselves
void Player::search(bool force_serial){
	
	clock::time_point real_start = clock::now();
	bool threaded = true;
	int lane_no=-1;
	moveGenerator my_moves(my_color, root);
	
	cout_lock.lock();
	node_count++;		//debug for root node
	cout_lock.unlock();
	
	
	if(yourBest == root){// let thread continue
		cout << "Expected board returned. Continuing search." << endl;
		think_deadline = move_deadline;
		goto continue_prediction;
	}else{// kill thread and start new ones
		cout << "Different board returned. Restarting search." << endl;
		think_deadline = clock::now();
	}
	
	board_lock.lock();
	global_alpha = -2.0; // before threads
	global_beta = 2.0;
	return_board_val = (float)(my_color == WHITE ? -2.0 : 2.0); //each itteration starts with the same values
	return_board = *my_moves; // assume first move is best until proven otherwise
	board_lock.unlock();

	
if(!force_serial){
	
	while (*my_moves){
		lane_no++;
		
//		cout_lock.lock();
//		cout << "(Master) Creating thread: " << lane_no << endl;//debug
//		cout_lock.unlock();

		cout_lock.lock();
		node_count++;		//debug for children nodes
		cout_lock.unlock();
		
		
		try{
			branches.create_thread(boost::bind(&Player::parallelSearch, this, lane_no, *my_moves));
//			branches.join_all();//debug - make serial
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
}else{
	threaded = false;
	lane_no = 12;
}
	if(!threaded){	// will try serial search
		cout << "(Master) Going serial" << endl;
		serialSearch();
		return;
	}
	
	bf.push_back(lane_no+1);
	
	if (lane_no <= 0 ){//only one move available
		goto only_move_cutoff;
	}

//	cout_lock.lock();
//	cout << "(Master) Branch Factor: " << lane_no+1 << endl;	//debug
//	cout_lock.unlock();

continue_prediction:
	while(clock::now() < move_deadline){
		// wait
	}
	
only_move_cutoff:
	move_deadline = clock::now();
	
//	clock::time_point real_stop = clock::now();
//	boost::chrono::duration<double>	lifespan = (real_stop - real_start);
//	cout_lock.lock(); 	
//	cout << "(Master) total time for search: " << lifespan  << endl;//debug
//	cout << "Counted nodes: " << node_count << endl;
//	cout_lock.unlock();
}

void Player::thinkAhead(){
	/*
	 creates new threads from rooted at your best
	 exits - no blocking
	 */
	
	
	bool threaded = true;
	
	moveGenerator your_moves(my_color, yourBest);
	board_lock.lock();
	global_alpha = -2.0; // before threads
	global_beta = 2.0;
	return_board_val = (float)(my_color == WHITE ? -2.0 : 2.0); //each itteration starts with the same values
	return_board = *your_moves; // assume first move is best until proven otherwise
	board_lock.unlock();
	
	int lane_no=-1;
	
	while (*your_moves){
		lane_no++;
		
		//		cout_lock.lock();
		//		cout << "(Master) Creating thread: " << lane_no << endl;//debug
		//		cout_lock.unlock();
		
		try{
			branches.create_thread(boost::bind(&Player::parallelSearch, this, lane_no, *your_moves));
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
			your_moves->printBoard();
			
			threaded = false;
			break;
		}
		catch (...) {
			cerr << "Unknown error" << cout;
		}
		your_moves++;
	}
	
	if(!threaded){	// will try serial search
		cout << "(Master) Going serial" << endl;
		serialSearch();
		return;
	}
	
	bf.push_back(lane_no+1);
	
	if (lane_no <= 0 ){//only one move available
		think_deadline = clock::now();
		return;
	}
	
	//	cout_lock.lock();
	//	cout << "(Master) Branch Factor: " << lane_no+1 << endl;	//debug
	//	cout_lock.unlock();
	
	while(clock::now() < think_deadline){
		// wait
	}	

}

// returns the current best board
board Player::getmove(){
	board_lock.lock();
	board return_copy = return_board;
	board_lock.unlock();
	return return_copy;
}


std::ostream & operator<<(std::ostream & theStream, const Player & p){
	theStream << "Age:		" << p.getAge() << endl;
	theStream << "Points:	" << p.getScore() << endl;
	return theStream;
}


void Player::save(boost::filesystem::path loc){
	boost::filesystem::ofstream file(loc);
	if (!file.is_open() || !file.good()){
		std::cerr << "Write Error" << std::endl;
		return;
	}
	file << evaluator;
	file.close();
}

bool Player::load(boost::filesystem::path loc){
	boost::filesystem::ifstream file(loc, std::ifstream::binary);
	if (!file.is_open() || !file.good()){
		cout << "Read Error" << endl;
		return false;
	}
	file >> evaluator;
	file.close();
	return true;
}

bool Player::operator<(const Player & other){
	return (this->getScore() < other.getScore());
}

/*
 mutate() 
 Pre:  Child is a valid player
 Post: Child is a mutant of caller
 */
void Player::mutate(Player & child){
	FFNN_mutate(child.evaluator, this->evaluator);
}