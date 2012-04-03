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

#include "Player.h"

#define POPULATION	50
#define SURV_THRESHOLD	(30.0*generation/(20.0+generation)+40.0)


class Training {
private:
	int generation;
	std::vector<Player*> competitors;
	
public:
	Training();
	~Training();
	void run();

	void save(std::string);
	bool load(std::string);
	
private:
	void orginization();
	void gameplay(Player &, Player &); // play a game between two player pointers
	void reproduction();
};

#endif // FILE_TRAINING_H_INCLUDED