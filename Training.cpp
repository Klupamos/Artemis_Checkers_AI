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

#define Save_Loc	std::string("/Users/greg/Dropbox/School/Current/CS_405/Artemis/Training_nets/")
#define	MOVETIME_EQUATION	(std::min(59.5,(1.5+generation))*1000.0)
#define CBUFFER	20

Training::Training():Avg_branching_factor_num(0) {
	generation = 0;

	competitors.resize(POPULATION);
	for(size_t i=0; i<competitors.size(); ++i){
		competitors[i] = new Player;
	}
	
	omp_init_lock(&locks);

	for(int i=0; i<POPULATION; ++i){
		omp_init_lock( plocks+i );
	}
	
};

static bool deleteAll( Player * theElement ) { delete theElement; return true; }
Training::~Training(){
	std::remove_if(competitors.begin(), competitors.end(), deleteAll);
	omp_destroy_lock(&locks);
	for(int i=0; i<POPULATION; ++i){
		omp_destroy_lock( plocks+i );
	}
}

void Training::save(string SaveLocation){
	// try to create Genreation Folder
	path genFolder(SaveLocation);
	stringstream ss;
	ss << generation;
	
	genFolder /= ss.str();
	
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

};
void Training::save_meta(string SaveLocation){

	path meta_loc(SaveLocation);
	stringstream ss;
	ss << generation;
	meta_loc /= ss.str();
	meta_loc /= "meta.txt";
	boost::filesystem::ofstream file(meta_loc);
	
	file << "Generation " << generation << endl;
	file << "Avg mpg:       " << Avg_moves_per_game_num / (2.0 * POPULATION) << endl;
	//	file << "Runtime: " << boost::chrono::duration<double, boost::chrono::minutes>(turny_end - turny_start) << endl;
	typedef boost::chrono::duration<double, boost::ratio<3600> > hours;
	hours hrs(turny_end - turny_start);
	file << "Runtime:       " << hrs << endl;
	file << "Avg game time: " << hrs / (POPULATION*2.0) << endl;
	file << "Avg bf:        " << (double)Avg_branching_factor_num / Avg_moves_per_game_num << endl;
	
	file << "-------------------" << endl << endl;
	
	int orig = 0;
	int cutoff = (int)(SURV_THRESHOLD / 100.0 * POPULATION);
	for (int net_number = 0; net_number < POPULATION; net_number++ ){
		
		if (net_number == cutoff){
			file << "-------------------" << endl << endl;
		}
		
		file << *(competitors[net_number]) << endl;
		if(competitors[net_number]->getUID().find("g") != std::string::npos){
			++orig;
		}
		
	}
	file << "-------------------" << endl;
	file << "Percent remaining from first gen: " << (double)orig/POPULATION*100.0 << "%" << endl;
	file.close();
};

bool Training::load(string SaveLocation){
	
	path genFolder;
	
	bool dir_exists = false;
	do{
		genFolder = path(SaveLocation);
		
		std::cout << "Enter the generation to load:   (0 to not load any players)" << std::endl;
		std::cin >> generation;

		if(generation == 0){
			return true;
		}

		// try to create Genreation Folder
		stringstream ss;
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
		Avg_branching_factor_num = 0;
		Avg_moves_per_game_num = 0;
		
		cout << "Starting generation " << ++generation << endl;
		cout << "\t" << MOVETIME_EQUATION/(1000.0) << " seconds per move" << endl;

		save(Save_Loc);

		turny_start = boost::chrono::steady_clock::now();
		tournament();	// game play happens here
		turny_end = boost::chrono::steady_clock::now();
		
		organization(); // Best first sort
		
		save_meta(Save_Loc);

		reproduction();	// mutate below cutoff / birthday above
	}while (true);
}

void Training::tournament(){

	#pragma omp parallel for
	for(int index=0; index<POPULATION; ++index){
		
		int opp1;
		int opp2;
		 
		boost::chrono::steady_clock::time_point start;
		boost::chrono::steady_clock::time_point end;
		
		// game 1
		omp_set_lock(&locks);
		omp_set_lock(&(plocks[index]));
		while (true){
			opp1 = (int)((POPULATION-1) * FFNN::randGen());
			
			if(index == opp1){
				continue;
			}
			if(!omp_test_lock(plocks + opp1)){
				continue;
			}
			
			break;
		}
		omp_unset_lock(&locks);
		
		start = boost::chrono::steady_clock::now();
		gameplay(*(competitors[index]), *(competitors[opp1]));
		end = boost::chrono::steady_clock::now();
		
		omp_unset_lock(&(plocks[opp1]));
		
		cout << "(" << omp_get_thread_num() << ") player: " << index << "\tTime: " << boost::chrono::duration<double>(end - start) << endl;
		
		
		
		// game 2
		omp_set_lock(&locks);
		while (true){	
			opp2 = (int)((POPULATION-1) * FFNN::randGen());
			
			if(index == opp2){
				opp2 = (int)((POPULATION-1) * FFNN::randGen());
				continue;
			}
			if(!omp_test_lock(plocks + opp2)){
				continue;
			}
			
			break;
		}
		omp_unset_lock(&locks);

		start = boost::chrono::steady_clock::now();
		gameplay(*(competitors[index]), *(competitors[opp2]));
		end = boost::chrono::steady_clock::now();
		
		omp_unset_lock(&(plocks[index]));
		omp_unset_lock(&(plocks[opp2]));
		
		cout << "(" << omp_get_thread_num() << ") player: " << index << "\tTime: " << boost::chrono::duration<double>(end - start) << endl;
	}
}

void Training::gameplay(Player & White, Player & Black){
	int moves=0;	
	bool cheated = false;
	milliseconds turn_time_limit((long long)MOVETIME_EQUATION);
	White.setColor(WHITE);
	White.setTimeLimit(turn_time_limit);
	Black.setColor(BLACK);
	Black.setTimeLimit(turn_time_limit);
	
	board cycle_buffer[CBUFFER];
	board officialBoard(0x00000FFF, 0xFFF00000, 0);
	color_t active_player = BLACK;
	
	stringstream game_log;
	
	game_log << officialBoard;
	
	int bf;
	do{
		++moves;
		for(int l=0; l<CBUFFER; ++l){
			for(int r=l+1; r<CBUFFER; ++r){
				if (cycle_buffer[l] == cycle_buffer[r]){
					break;//two equal boards found in the buffer
				}
			}
		}
		
		if(active_player == WHITE){
			game_log << "White's move"<<endl;
			bf = White.newboard(officialBoard);
			if(!bf){
				game_log << "White calls cheater"<<endl;
				cout << "White calls cheater"<<endl;
				cheated = true;
				game_log << Black.splat();
				White.victory();
				Black.defeat();
				goto save_game_log;
			}
			White.search();
			officialBoard = White.getmove();
//			White.thinkAhead();
		}else{
			game_log << "Black's move"<<endl;
			bf = Black.newboard(officialBoard);
			if(!bf){
				game_log << "Black calls cheater"<<endl;
				cout << "Black calls cheater"<<endl;
				cheated = true;
				game_log << White.splat();
				Black.victory();
				White.defeat();
				goto save_game_log;
			}
			Black.search();
			officialBoard = Black.getmove();
//			Black.thinkAhead();
		}
		
		game_log << officialBoard;
		
		Avg_branching_factor_num += bf;
		
		if(officialBoard == board()){	//active Player could not make a move
			if(active_player == WHITE){
				Black.victory();
				White.defeat();
			}else{
				Black.defeat();
				White.victory();
			}
			goto save_game_log;
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
	
save_game_log:
	Avg_moves_per_game_num += moves;
	
	path genFolder(Save_Loc);
	stringstream ss;
	ss << generation;
	genFolder /= ss.str();
	
	ss.str(std::string());
	ss << (cheated == true? "Review_": "") << "Game_" << White.getUID() << "_vs_" << Black.getUID() << ".log";
	genFolder /= ss.str();
	
	boost::filesystem::ofstream file(genFolder);
	file << game_log.str() << endl;
	file.close();
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
