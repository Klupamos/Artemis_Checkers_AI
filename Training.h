/*
 *  Training.h
 *  Artemis_Checkers_AI
 *
 *  Created by Gregory Klupar on 4/1/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_TRAINING_H_INCLUDED
#define FILE_TRAINING_H_INCLUDED

#include <string>

#include <omp.h>

#include "Player.h"

#define POPULATION	50
#define SURV_THRESHOLD	(30.0*generation/(20.0+generation)+40.0)


class Training {
private:
	int generation;
	std::vector<Player*> competitors;
	
	omp_lock_t locks;
	omp_lock_t* plocks; // locks for each player	
	
	double Average_expected_board;
	double Average_branching_factor;
	double Average_moves_per_game;
	
public:
	Training();
	~Training();
	void run();

	void save(std::string);
	bool load(std::string);
	
private:

	void tournament();
	void organization();
	void gameplay(Player &, Player &);
	void reproduction();
};



#endif // FILE_TRAINING_H_INCLUDED