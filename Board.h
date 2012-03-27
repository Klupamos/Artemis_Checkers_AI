/*
 *  Board.h
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_BOARD_H_INCLUDED
#define FILE_BOARD_H_INCLUDED

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
	
	void printBoard() const;
	bool winner() const;

	inline	operator bool() const{
		return (whitePawns | blackPawns) > 0;
	};
	
	inline	bool operator==(const board & rhs){
		return (this->whitePawns == rhs.whitePawns && this->blackPawns == rhs.blackPawns && this->kings == rhs.kings);
	};
	
	inline	bool operator!=(const board & rhs){
		return !(*this == rhs);
	};
	
};

#endif	// FILE_BOARD_H_INCLUDED