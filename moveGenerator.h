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

#include "Board.h"
#include "Piece.h"

#include <stdint.h>
typedef uint32_t	ULONG;



class moveGenerator{
public:
	enum {
		MASK_F3 = 0x00E0E0E0,
		MASK_F5 = 0x07070707,
		MASK_B3 = 0x07070700,
		MASK_B5 = 0xE0E0E0E0,
		MASK_BACK_ROW = 0xF000000F
	};
	
	enum Direction {F4, F35, B4, B35, INC};
	
private:
	piece_t player;
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
	moveGenerator(piece_t, board);
	
	board curBoard();
	void nextBoard();	
		
	board& operator*();
	board* operator->();
	moveGenerator& operator++();
	moveGenerator operator++(int);
	
private:
public://debug
	ULONG sliders(piece_t);
	ULONG jumpers(piece_t);
};


#endif //FILE_MOVE_GENERATOR_H_INCLUDED