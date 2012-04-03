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
#define MAX_BF	50

class Player { // can also be thought of as the player
public:
	typedef boost::chrono::steady_clock	clock;
	typedef boost::chrono::seconds seconds;
	
	enum {WIN = 1, DRAW = 0, LOSS = -2};
private:
	int turny_points;
	int age;
	
	color_t my_color;
	FFNN evaluator;
	board root;
	
	clock::time_point move_deadline;
	clock::time_point think_deadline;
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
	float return_board_val;
	board return_board;
	
	
public:
	Player();
	void setColor(color_t);
	void mutate(Player &);
	
	bool newboard(const board &);
	void search(bool force_serial = false); // does searching for my move
	void thinkAhead();	
	board getmove();
	board getyourmove(){return yourBest;};//debug
	
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
