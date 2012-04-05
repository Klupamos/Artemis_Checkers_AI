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
using std::string;
#include <sstream>
using std::stringstream;

#include <algorithm>
using std::sort;

#include <boost/chrono.hpp>
using boost::chrono::milliseconds;

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
using boost::filesystem::path;
using boost::filesystem::is_directory;
using boost::filesystem::create_directories;
using boost::filesystem::exists;

#include <omp.h>


#include "FFNN.h"	//for FFNN::randGen()
#include "Player.h"
#include "Training.h"

#define Save_Loc	string("/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/")
#define	MOVETIME_EQUATION	(std::min(59.5,(1.5+generation))*1000.0)

Training::Training() {
	generation = 0;

	competitors.resize(POPULATION);
	for(size_t i=0; i<competitors.size(); ++i){
		competitors[i] = new Player;
	}
	
	omp_init_lock(&locks);

	plocks = new omp_lock_t[POPULATION];
	for(int i=0; i<POPULATION; ++i){
		omp_init_lock( &(plocks[i]) );
	}
	
};

static bool deleteAll( Player * theElement ) { delete theElement; return true; }
Training::~Training(){
	std::remove_if(competitors.begin(), competitors.end(), deleteAll);
	omp_destroy_lock(&locks);
	for(int i=0; i<POPULATION; ++i){
		omp_destroy_lock( &(plocks[i]) );
	}
	delete[] plocks;
}

void Training::save(string SaveLocation){
	// try to create Genreation Folder
	path genFolder(SaveLocation);
	stringstream ss;
	ss.clear();
	ss << generation;
	
	genFolder /= ss.str();
	cout << genFolder << endl;
	
	//try to create SaveLoaction
	if (!is_directory(genFolder)){
		create_directories(genFolder);
	}
	
	
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		path netPath(genFolder);
		stringstream tmp;
		tmp << net_number;
		
		netPath /= tmp.str();
		
		//try to create network file
		if(!boost::filesystem::is_regular(netPath)){
			competitors[net_number]->save(netPath);
			
		}
	}

	genFolder /= "meta.txt";
	boost::filesystem::ofstream file(genFolder);
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		file << *(competitors[net_number]) << endl;
	}

};

bool Training::load(string SaveLocation){
	
	path genFolder;
	
	bool dir_exists = false;
	do{
		genFolder = path(SaveLocation);
		
		cout << "Enter the generation to load: ";
		std::cin >> generation;
		
		// try to create Genreation Folder
		stringstream ss;
		ss.clear();
		ss << generation;
		genFolder /= ss.str();

		dir_exists = is_directory(genFolder);
	}
	while(!dir_exists);
	
	cout << "Loading from generation " << generation << endl;
	
	
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		path netPath(genFolder);
		stringstream tmp;
		tmp << net_number;
		
		netPath /= tmp.str();
		
		if(!boost::filesystem::exists(netPath) || !competitors[net_number]->load(netPath)){
			return false;
		}

		
	}
	
	return true;
};

void Training::run(){
	do{
		tournament();	// game play happens here
		organization(); // Best first sort

		//if(generation % 5 == 0){
			save(Save_Loc);
		//}
		
		reproduction();	// mutate below cutoff / birthday above

	}while (true);
}

void Training::tournament(){
	++generation;

	#pragma omp parallel for
	for(int index=0; index<POPULATION; ++index){
		
		
		
		int opp1 = (int)((POPULATION-1) * FFNN::randGen());
		int opp2 = (int)((POPULATION-1) * FFNN::randGen());
		
		if (opp1 == index){++opp1;} // playing self wont work
		if (opp2 == index){++opp2;} // 
		
		// game 1
		omp_set_lock(&locks);
		omp_set_lock(&(plocks[index]));
		omp_set_lock(&(plocks[opp1]));
		omp_unset_lock(&locks);
		
		cout << "(" << omp_get_thread_num() << ") Gen: " << generation << "\t| " <<  index << " playing " << opp1 << endl;
		gameplay(*(competitors[index]), *(competitors[opp1]));


		omp_unset_lock(&(plocks[index]));
		omp_unset_lock(&(plocks[opp1]));
		
		
		// game 2
		omp_set_lock(&locks);
		omp_set_lock(&(plocks[index]));
		omp_set_lock(&(plocks[opp2]));
		cout << "(" << omp_get_thread_num() << ") Gen: " << generation << "\t| " <<  index << " playing " << opp2 << endl;
		omp_unset_lock(&locks);

		gameplay(*(competitors[index]), *(competitors[opp2]));
		omp_unset_lock(&(plocks[index]));
		omp_unset_lock(&(plocks[opp2]));
		
	}
	
}

void Training::gameplay(Player & White, Player & Black){
	size_t moves=0;	
	
	milliseconds turn_time_limit((long long)MOVETIME_EQUATION);
	White.setColor(WHITE);
	White.setTimeLimit(turn_time_limit);
	Black.setColor(BLACK);
	Black.setTimeLimit(turn_time_limit);
	
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
			White.search();// do serial search (I dont trust alpha-beta)
			officialBoard = White.getmove();
			White.thinkAhead();
		}else{
			if(!Black.newboard(officialBoard)){
				Black.victory();
				White.defeat();
				return;
			}
			Black.search();
			officialBoard = Black.getmove();
			Black.thinkAhead();
		}
		
		if(officialBoard == board()){	//active Player could not make a move
			if(active_player == WHITE){
				Black.victory();
				White.defeat();
			}else{
				Black.defeat();
				White.victory();
			}
			return;
		}
		
		active_player = !active_player;
	}while (!officialBoard.winner());
	
	if(officialBoard.whitePawns && !officialBoard.blackPawns){
		White.victory();
		Black.defeat();
	}else if (officialBoard.blackPawns && !officialBoard.whitePawns){
		Black.victory();
		White.defeat();
	}else{
		White.draw();
		Black.draw();
	}
	
}

bool lessThan(Player* i,Player* j) { return (i->getScore() > j->getScore()); }

void Training::organization(){
	sort(competitors.begin(), competitors.end(), lessThan);// now in Best first order
}

void Training::reproduction(){
	
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
