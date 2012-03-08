/*
 *  moveGenerator.h
 *  Artemis
 *
 *  Created by Gregory Klupar on 2/6/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_MOVE_GENERATOR_H_INCLUDED
#define FILE_MOVE_GENERATOR_H_INCLUDED

typedef unsigned long	ULONG;

class board{
public:
	ULONG whitePawns;
	ULONG blackPawns;
	ULONG kings;
	
	/*
	   00  01  02  03	White starts on top
	 04  05  06  07		  and moves down
	   08  09  10  11		
	 12  13  14  15			
	   16  17  18  19		
	 20  21  22  23		Black starts on bottom
	   24  25  26  27	  and moves up
	 28  29  30  31	  */
	
	board():whitePawns(0), blackPawns(0), kings(0){};
	board(ULONG w, ULONG b, ULONG k):whitePawns(w), blackPawns(b), kings(k){};
	
	void printBoard();
	
	friend bool operator==(const board & lhs, const board & rhs);
	friend bool operator!=(const board & lhs, const board & rhs);
	
};


class moveGenerator{
public:
	enum {
		MASK_D5 = 0xE0E0E0E0,
		MASK_D3 = 0x07070707,
		MASK_U3 = MASK_D5,
		MASK_U5 = MASK_D3
	};
	
	enum player_t {WHITE, BLACK};
	
	enum Direction{NONE,P4,PO,M4,MO};
	
private:
	player_t player;
	board main_Board;
	board current_Board;
	ULONG sliding_peices;	// only peices that can slide
	ULONG jumping_peices;	// only peices that can jump
	ULONG current_peice;	// the currently selected peice
	ULONG my_peices;		// all peices that can move (sliding_peices XOR jumping_peices)
	ULONG your_peices;
	Direction direction_tested;
	
public:
	moveGenerator();
	moveGenerator(player_t, board);
	
	board curBoard();
	void nextBoard();	
	
	board & operator*();
	moveGenerator & operator++();
	moveGenerator operator++(int);
	
private:

	ULONG sliders(player_t);
	ULONG jumpers(player_t);
};


#endif //FILE_MOVE_GENERATOR_H_INCLUDED