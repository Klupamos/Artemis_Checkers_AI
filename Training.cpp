/*
 *  Training.cpp
 *  Artemis_Checkers_AI
 *
 *  Created by Gregory Klupar on 4/1/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */
#include <iostream>
using std::cout;
using std::endl;

#include <string>
#include <sstream>

#include <algorithm>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using namespace boost::filesystem;

#include "FFNN.h"	//for FFNN::randGen()
#include "Player.h"
#include "Training.h"

#define Save_Loc	std::string("/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/")


bool lessThan(Player* i,Player* j) { return (i->getScore() > j->getScore()); }

Training::Training() {
	generation = 0;

	competitors.resize(POPULATION);
	for(size_t i=0; i<competitors.size(); ++i){
		competitors[i] = new Player;
	}
};

static bool deleteAll( Player * theElement ) { delete theElement; return true; }
Training::~Training(){
	std::remove_if(competitors.begin(), competitors.end(), deleteAll);
}

void Training::save(std::string SaveLocation){
	// try to create Genreation Folder
	path genFolder(SaveLocation);
	std::stringstream ss;
	ss.clear();
	ss << generation;
	
	genFolder /= ss.str();
	cout << genFolder << endl;
	
	//try to create SaveLoaction
	if (!is_directory(genFolder)){
		cout << "Creating Generation Directory " << genFolder << endl;
		create_directories(genFolder);
	}
	
	
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		path netPath(genFolder);
		std::stringstream tmp;
		tmp << net_number;
		
		netPath /= tmp.str();
		
		cout << "creating file " << netPath  << endl;
		//try to create network file
		if(!is_regular(netPath)){
			competitors[net_number]->save(netPath);
			
		}
	}
};

bool Training::load(std::string SaveLocation){
	// try to create Genreation Folder
	path genFolder(SaveLocation);
	std::stringstream ss;
	ss.clear();
	ss << generation;
	
	genFolder /= ss.str();
	cout << genFolder << endl;
	
	//try to create SaveLoaction
	if (!is_directory(genFolder)){
		return false;
	}
		
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		path netPath(genFolder);
		std::stringstream tmp;
		tmp << net_number;
		
		netPath /= tmp.str();
		
		//try to create network file
		if(!exists(netPath) || !competitors[net_number]->load(netPath)){
			return false;
		}

		
	}
	
	genFolder /= "meta.txt";
	boost::filesystem::ofstream file(genFolder);
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		file << "NN: " << net_number << endl;
		file << competitors[net_number] << endl;
		
	}
	
	return true;
};

void Training::run(){

	orginization();
	reproduction();
	
	if(generation % 5 == 0){
		save(Save_Loc);
	}
	
}

void Training::orginization(){
	++generation;
	
	for(int index=0; index<POPULATION; ++index){
		
		
		int opp1 = (int)(POPULATION * FFNN::randGen());
		int opp2 = (int)(POPULATION * FFNN::randGen());
		
		// game 1
		cout << index << " playing " << opp1 << endl;
		//gameplay(*(competitors[index]), *(competitors[opp1]));

		// game 2
		cout << index << " playing " << opp2 << endl;
		//gameplay(*(competitors[index]), *(competitors[opp2]));
	}
	
}

void Training::gameplay(Player & White, Player & Black){
	size_t moves=0;	
	
	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	color_t active_player = WHITE;
	do{
		++moves;
		if(moves >= 80){
			break;
		}
		
		if(active_player == WHITE){
			if(!White.newboard(officialBoard)){
				White.victory();
				Black.defeat();
				return;
			}
			White.search();
			officialBoard = White.getmove();
		}else{
			if(!Black.newboard(officialBoard)){
				Black.victory();
				White.defeat();
				return;
			}
			Black.search();
			officialBoard = Black.getmove();
		}
		active_player = !active_player;
	}while (!officialBoard.winner());
	
	
	if(officialBoard.whitePawns){
		White.victory();
		Black.defeat();
	}else if (officialBoard.blackPawns){
		Black.victory();
		White.defeat();
	}else{
		White.draw();
		Black.draw();
	}
}

void Training::reproduction(){
		
	std::sort(competitors.begin(), competitors.end(), lessThan);// now in Best first order
	
	int cutoff = (int)(SURV_THRESHOLD / 100.0 * POPULATION);
	
	for(int index = 0; index < cutoff; ++index){
		competitors[index]->resetScore();
		competitors[index]->birthday();
	}
	
	
	for(int index = cutoff; index < POPULATION; ++index){
		
		int parent_index = (int)(cutoff * FFNN::randGen());
		
		competitors[parent_index]->mutate(*(competitors[index])); // wrapper for FFNN_mutate
		
		competitors[index]->resetScore();
		competitors[index]->resetAge();
	}
}
