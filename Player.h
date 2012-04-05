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

#include <iostream>
#include <string>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include "FFNN.h"
#include "Board.h"
#include "MoveGenerator.h"
#include "Color.h"

// largest seen in testing was 11
#define MAX_BF	30

class Player { // can also be thought of as the player
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::milliseconds milliseconds;
	
	enum point_t {WIN = 1, DRAW = 0, LOSS = -2};
	enum state_t {DOING, THINKING};
private:
	std::string uid;
	int turny_points;
	int age;
	
	color_t my_color;
	FFNN evaluator;
	board root;
	
	milliseconds move_time_limit;
	clock::time_point move_deadline;
	boost::thread_group branches;
	board yourmoves[MAX_BF];
	boost::mutex cout_lock;// look at upgrade_locks and shared_locks	
	
	int node_count;
	std::vector<int> bf;
	
	boost::mutex your_lock;
	board yourBest;
	
	boost::mutex alpha_lock;
	float global_alpha;

	boost::mutex beta_lock;
	float global_beta;

	boost::mutex board_lock;
	float buffer_board_val;
	board buffer_board;
	
	board return_board;
	
	
public:
	Player();
	~Player(){branches.join_all();}
	void setTimeLimit(boost::chrono::milliseconds);
	void setColor(color_t);
	void mutate(Player &);
	
	bool newboard(const board &);
	void search(); // does searching for my move
	void thinkAhead();	// will uses opponents time to search for my next move
	board getmove();
	board getyourmove(){return yourBest;};//debug
	
	const std::string & getUID()const{return uid;}
	void victory(){turny_points+=WIN;}
	void defeat(){turny_points+=LOSS;}
	void draw(){turny_points+=DRAW;}
	int getScore()const{return turny_points;}
	void resetScore(){turny_points = 0;}
	int getAge()const{return age;}
	void resetAge(){age = 0;}
	void birthday(){++age;}
	
	void save(boost::filesystem::path);
	bool load(boost::filesystem::path);
	
	bool operator<(const Player &);

private:
	void startTimmer();
	
	void parallelSearch(int, const board & );
	void serialSearch();
	
	float bestBoard(char, color_t, const board &, int, float, float);
	float worstBoard(char, color_t, const board &, int, float, float);
	bool terminal(const board &, int);


friend std::ostream & operator<<(std::ostream &, const Player &);
};




#endif // FILE_Player_H_INCLUDED
