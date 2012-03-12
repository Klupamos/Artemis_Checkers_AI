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
	  31  30  29  28	Black starts on top
	27  26  25  24		  and moves down
	  23  22  21  20
	19  18  17  16
	  15  14  13  12
	11  10  09  08
	  07  06  05  04	White starts on bottom
	03  02  01  00		  and moves up
	*/
	
	
	board():whitePawns(0), blackPawns(0), kings(0){};
	board(ULONG w, ULONG b, ULONG k):whitePawns(w), blackPawns(b), kings(k){};
	
	void printBoard();
	
	friend bool operator==(const board & lhs, const board & rhs);
	friend bool operator!=(const board & lhs, const board & rhs);
	
};


class moveGenerator{
public:
	enum {
		/*
		MASK_D5 = 0xE0E0E0E0,
		MASK_D3 = 0x07070707,
		MASK_U3 = MASK_D5,
		MASK_U5 = MASK_D3,
		
		MASK_F5 = MASK_D3,
		MASK_F3 = MASK_D5,
		MASK_B5 = MASK_D5,
		MASK_B3 = MASK_D3
		 */
		
		// VV these correct VV
		MASK_F3 = 0x00E0E0E0,
		MASK_F5 = 0x07070707,
		MASK_B3 = 0x07070700,
		MASK_B5 = 0xE0E0E0E0
	};
	
	enum player_t {WHITE, BLACK};
	
	enum Direction{F4, F35, B4, B35, INC};
	
private:
	player_t player;
	board main_Board;
	
	char stack_pos;
	board current_Board[10];
	ULONG current_Piece[10];	// the currently selected piece
	Direction next_Test[10];
	bool end_of_chain[10];
	
	ULONG sliding_pieces;	// only pieces that can slide
	ULONG jumping_pieces;	// only pieces that can jump
	
	ULONG my_movable_pieces; // all pieces that can move (sliding_pieces XOR jumping_pieces)
	ULONG* my_pieces;		 //	if (player == WHITE) points to current_Board.whitepawns
	ULONG* your_pieces;		 //	if (player == WHITE) points to current_Board.blackpawns
	
	
	
public:
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