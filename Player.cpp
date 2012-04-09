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


Player::Player():age(0), turny_points(0), Avg_eb_hit_num(0), Avg_eb_hit_den(0){
	FFNN_setup(&evaluator);

	union {
		float f;
		char ch[4];
	}tmp;

	tmp.f = (evaluator.king_value);
	
	std::stringstream ss;
	ss << std::hex << (0x0FF & tmp.ch[0]) << (0x0FF & tmp.ch[1])  << (0x0FF & tmp.ch[2])  << (0x0FF & tmp.ch[3]) ;
	
	uid = ss.str(); // king value for neural network
}


void Player::setColor(color_t color){
	my_color = color;
	// clear any modifications the last game made to Player
	branches.interrupt_all();
	root = board(0x00000FFF, 0xFFF00000, 0);
	buffer_board = return_board = board();
	branches.join_all();
}

int Player::newboard(const board & newb){
	startTimmer();
	
	root = newb;
	
	if(return_board == board()){
		return 7;
	}
	
	bool valid = false;
	int move_count = 0;
	
	moveGenerator nextmove(!my_color, return_board);
	while (*nextmove) {
		if (*nextmove == newb){
			valid = true;
		}
		nextmove++;
		move_count++;
	}
	
	if(valid){
		return move_count;
	}
	return 0;
	
}

void Player::startTimmer(){ 
	move_deadline = clock::now() + move_time_limit;
}

void Player::serialSearch(){	
	int current_height = search_start_height;
	
	try {
		global_alpha = -2.0;
		global_beta = 2.0;
		buffer_board_val = (float)(my_color == WHITE ? -2.0 : 2.0 ); //each itteration starts with the same value

		while (true){
			moveGenerator mg(my_color, root);
			
			while (*mg){
				
				if(my_color == WHITE){

					global_alpha = max(global_alpha, worstBoard(0, !my_color, *mg, current_height-1, global_alpha, global_beta));

					if(global_alpha > buffer_board_val){
						buffer_board_val = global_alpha;
						buffer_board = *mg;
						yourBest = yourmoves[0];	// save What I think your move will be
					}
				}else{
					global_beta = min(global_beta, bestBoard(0, !my_color, *mg, current_height-1, global_alpha, global_beta));
					if(global_beta < buffer_board_val){
						buffer_board_val = global_beta;
						buffer_board = *mg;

						yourBest = yourmoves[0];	// save What I think your move will be
					}
				}
				
				mg++;
				
			}
			current_height++;
		}
	}
	catch (int e) {
		if(e!=-1){
			cerr << "Unknow error: " << e << endl;
			throw e;
		}
	}
//stats section
	if(current_height > search_start_height &&  current_height < MODAL_SIZE + search_start_height ){
		stats_lock.lock();
		++modal_bucket[current_height-1 -search_start_height];
		stats_lock.unlock();
	}

}

float Player::bestBoard(char lane, color_t p,const board & b, int current_height, float alpha, float beta){
		// I am terminal, so return my value
		if (terminal(b, current_height)){
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		board tmp;
		float omega;
		//not a terminal board, so find the max of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			omega = max(alpha, worstBoard(-1, !p, *nextmove, current_height-1, alpha, beta));
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
			return FFNN_calculateOutputs(&evaluator, b, p);
		}
		
		board tmp;
		float omega;
		//not a terminal board, so find the min of my children
		moveGenerator nextmove(p, b);
		while (*nextmove) {
			omega = min(beta, bestBoard(-1, !p, *nextmove, current_height-1, alpha, beta));
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
	
	if(clock::now() >= move_deadline){ //ran out of time
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
		if (tptr.get() == NULL){
			tptr.reset(new clock::time_point(move_deadline));
		}
		
		
		while(true){
		
			moveGenerator mg(!my_color, my_root);
			while (*mg){
				if(boost::this_thread::interruption_requested() || clock::now() >= *tptr){
					goto stats_place;
					return;
				}
				
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
			
			if(boost::this_thread::interruption_requested() || clock::now() >= *tptr){
				goto stats_place;
				return;
			}
			

			// This is what would be in the top most node
			if(my_color == WHITE){ // Max node
			
				tmp_omega = max(local_alpha, local_beta);
			
				alpha_lock.lock();
				if (tmp_omega > local_alpha){local_alpha = tmp_omega;}
				alpha_lock.unlock();

				board_lock.lock();
				if(tmp_omega > buffer_board_val && clock::now() < *tptr){
					buffer_board_val = tmp_omega;
					buffer_board = my_root;

					yourBest = yourmoves[lane_no];	// save What I think your move will be
				}
				board_lock.unlock();
			}else{ // Min node
				tmp_omega = min(local_beta, local_alpha);
			
				beta_lock.lock();
				if (tmp_omega < local_beta){local_beta = tmp_omega;}
				beta_lock.unlock();

				board_lock.lock();
				if(tmp_omega < buffer_board_val && clock::now() < *tptr){
					buffer_board = my_root;
					buffer_board_val = tmp_omega;

					yourBest = yourmoves[lane_no];	// save What I think your move will be
				}
				board_lock.unlock();
			}

			++current_height;
		}	
	}
	catch (int e) {
		if (e == -1){
			goto stats_place;
			return;
		}else{
			cerr << lane_no << " Unknown error: " << e << endl;
			throw e;
		}
	}

stats_place:
	if(current_height > search_start_height &&  current_height < MODAL_SIZE + search_start_height ){
		stats_lock.lock();
		++modal_bucket[current_height-1 -search_start_height];
		stats_lock.unlock();
	}
	return;
}

// sets up threads
// All threads created herein will be closed at function exit
void Player::search(){
	bool threaded = true;
	int lane_no=-1;
	moveGenerator my_moves(my_color, root);

/*	//needs to be commented if not calling thinkahead()
	if(yourBest == root){// let thread continue
//		cout << "Expected board returned. Continuing search." << endl;	//debug	
		Avg_eb_hit_num++;
		Avg_eb_hit_den++;
		
		goto continue_prediction;
	}else{// kill thread and start new ones
//		cout << "Different board returned. Restarting search." << endl;	//debug	
		Avg_eb_hit_den++;
		branches.interrupt_all();
	}
/**/
	
	board_lock.lock();
	global_alpha = -2.0; // before threads
	global_beta = 2.0;
	buffer_board_val = (float)(my_color == WHITE ? -2.0 : 2.0); //each itteration starts with the same values
	buffer_board = *my_moves; // assume first move is best until proven otherwise

	board_lock.unlock();
	
	while (*my_moves){
		lane_no++;
		
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

	if(!threaded){	// will try serial search
		cout << "(Master) going serial" << endl;
		branches.interrupt_all();
		serialSearch();
		return;
	}
	
	if (lane_no <= 0 ){//only one move available
		goto one_move_cutoff;
	}

continue_prediction:
	while(clock::now() < move_deadline){
		// wait
	}
	
one_move_cutoff:
	move_deadline = clock::now();
	branches.interrupt_all();
	
}

void Player::thinkAhead(){
	/*
	 creates new threads rooted at your best
	 exits - no blocking
	 */
	
	bool threaded = true;
	
	moveGenerator your_moves(my_color, yourBest);
	board_lock.lock();
	global_alpha = -2.0; // before threads
	global_beta = 2.0;
	buffer_board_val = (float)(my_color == WHITE ? -2.0 : 2.0); //each itteration starts with the same values
	buffer_board = *your_moves; // assume first move is best until proven otherwise
	board_lock.unlock();
	
	int lane_no=-1;
	
	while (*your_moves){
		lane_no++;
		
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
	
	if(!threaded){ // serial will block so just close threads
		branches.interrupt_all();
		return;
	}	
	
	if (lane_no <= 0 ){//only one move available
		branches.interrupt_all();
		return;
	}
	
}

// returns the current best board
board Player::getmove(){
	board_lock.lock();
	return_board = buffer_board;
	board_lock.unlock();
	return return_board;
}


std::ostream & operator<<(std::ostream & theStream, const Player & p){
	int mode=0;
	int hist=0;
	theStream << "UID:       " << p.getUID() << endl;
	theStream << "Points:    " << p.getScore() << endl;
	theStream << "Age:       " << p.getAge() << endl;

	for(int i=0;i<MODAL_SIZE-1; ++i){// ignore the last bucket, thats where we store all the overflow
			if (hist < p.modal_bucket[i]){
				hist = p.modal_bucket[i];
				mode = i;
			}
		}

	theStream << "Avg depth: " << mode + search_start_height << endl;
//	theStream << "Avg eb hit:" << (double)p.Avg_eb_hit_num / p.Avg_eb_hit_den << endl;	
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
	
	
	union {
		float f;
		char ch[4];
	}tmp;
	tmp.f = (evaluator.king_value);
	
	std::stringstream ss;
	ss << std::hex << (0x0FF & tmp.ch[0]) << (0x0FF & tmp.ch[1])  << (0x0FF & tmp.ch[2])  << (0x0FF & tmp.ch[3]) ;
	uid = ss.str(); // king value for neural network
	
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
	child.uid = uid + "g";
}

void Player::setTimeLimit(boost::chrono::milliseconds ms){
	move_time_limit = ms;
}


std::string Player::splat(){
	std::stringstream rs;
	rs << "Branches: " << branches.size() << endl;
	rs << "Root Board" << endl;
	rs << root;
	
	rs << "Buffer Board" << endl;
	rs << buffer_board;
	
	rs << "Return Board" << endl;
	rs << return_board;
	return rs.str();
}
