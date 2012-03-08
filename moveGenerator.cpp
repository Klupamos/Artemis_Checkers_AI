/*
 *  moveGenerator.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 2/6/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include <iostream>
using std::cout; using std::endl;

#include "moveGenerator.h"


void board::printBoard(){
	cout << "Board" << endl;
	
	ULONG WK = kings & whitePawns;
	ULONG WP = whitePawns;
	ULONG BK = kings & blackPawns;
	ULONG BP = blackPawns;
	
	for(int i=0; i<32; i++){
		if (i%8==0)
			cout << "  ";
		
		if( WK & (1<<i)){
			cout << "WK";
		}else if( WP & (1<<i)){
			cout << "WW";
		}else if( BK & (1<<i)){
			cout << "BK";
		}else if( BP & (1<<i)){
			cout << "BB";
		}else{
			cout << "--";
		}
		cout << "  ";
		
		if (i%4==3)
			cout << endl;
	}
	cout << endl << endl;
}

bool operator==(const board & lhs, const board & rhs){
	return (lhs.whitePawns == rhs.whitePawns && lhs.blackPawns == rhs.blackPawns && lhs.kings == rhs.kings);
};
bool operator!=(const board & lhs, const board & rhs){
	return !(lhs == rhs);
};



moveGenerator::moveGenerator():current_peice(1), direction_tested(NONE), player(WHITE){}
moveGenerator::moveGenerator(player_t p, board newBoard):current_peice(1), direction_tested(NONE), player(p), main_Board(newBoard), current_Board(newBoard){
	sliding_peices = sliders(player);
	jumping_peices = jumpers(player);
	
	if(player == WHITE){
		my_peices = current_Board.whitePawns;
		your_peices = current_Board.blackPawns;
	}else{
		my_peices = current_Board.blackPawns;	
		your_peices = current_Board.whitePawns;
	} 
	// my_peices = ( player == WHITE ? current_Board.whitePawns : current_Board.blackPawns);
	
	
	my_peices = my_peices & ((jumping_peices) | (sliding_peices) * (jumping_peices==0));
	// my_peices = jumping_peices OR my_peices = sliding_peices
		
	
	while (current_peice && !(current_peice & my_peices)){
		current_peice = (current_peice << 1);
	}
	
	nextBoard(); //set first move
}


/*
moveGenerator::ULONG moveGenerator::whiteMovers(){
	const ULONG openSquares = ~(whitePawns | blackPawns); // Not Occupied
	const ULONG whiteKings = whitePawns&kings;         // Kings
 	ULONG movers = 0;

	movers |= (openSquares >> 4) & whitePawns;
	movers |= ((openSquares & MASK_D5) >> 5) & whitePawns;
	movers |= ((openSquares & MASK_D3) >> 3) & whitePawns;
	if ( whiteKings ) {
		movers |= (openSquares << 4) & whiteKings;
		movers |= ((openSquares & MASK_U3) << 3) & whiteKings;
		movers |= ((openSquares & MASK_U5) << 5) & whiteKings;
	}
	
	return movers;
}
moveGenerator::ULONG moveGenerator::whiteJumpers(){
	const ULONG openSquares = ~(whitePawns | blackPawns);
	const ULONG whiteKings = whitePawns&kings;
 	ULONG movers = 0;
 	ULONG tmp = 0;
	// open back 4 == black; then  back (3|5) == white;
	// then open is a space to jump to;
	
	tmp = (openSquares >> 4) & blackPawns;
	if (tmp){
		movers |= ((tmp & MASK_D3) >> 3) & whitePawns;
		movers |= ((tmp & MASK_D5) >> 5) & whitePawns;
	}
	
	// open back (3|5) == black; then  back 4 == white;
	// then open is a space to jump to;
	
	tmp  =  ((openSquares & MASK_D3) >> 3) & blackPawns;
	tmp |=  ((openSquares & MASK_D5) >> 5) & blackPawns;
	movers |= (tmp >> 4) & whitePawns;
	
	if (whiteKings){
		tmp = (openSquares<< 4) & blackPawns;
		if (tmp){
			movers |= ((tmp&MASK_U3) << 3) & whiteKings;
			movers |= ((tmp&MASK_U5) << 5) & whiteKings;
		}
		tmp =  ((openSquares&MASK_U3) << 3) & blackPawns;
		tmp |= ((openSquares&MASK_U5) << 5) & blackPawns;
		if (tmp){
			movers |= (tmp << 4) & whiteKings;
		}
	}
	
	return movers;
}

moveGenerator::ULONG moveGenerator::blackMovers(){
	const ULONG openSquares = ~(blackPawns | whitePawns); // Not Occupied
	const ULONG blackKings = blackPawns&kings;         // Kings
 	ULONG movers = 0;
	
	movers |= (openSquares << 4) & blackPawns;
	movers |= ((openSquares & MASK_U5) << 5) & blackPawns;
	movers |= ((openSquares & MASK_U3) << 3) & blackPawns;
	if ( blackKings ) {
		movers |= (openSquares >> 4) & blackKings;
		movers |= ((openSquares & MASK_D3) >> 3) & blackKings;
		movers |= ((openSquares & MASK_D5) >> 5) & blackKings;
	}
	
	return movers;
}
moveGenerator::ULONG moveGenerator::blackJumpers(){
	const ULONG openSquares = ~(blackPawns | whitePawns);
	const ULONG blackKings = blackPawns&kings;
 	ULONG movers = 0;
 	ULONG tmp = 0;
	// open back 4 == black; then  back (3|5) == white;
	// then open is a space to jump to;
	
	tmp = (openSquares << 4) & whitePawns;
	if (tmp){
		movers |= ((tmp & MASK_D3) << 3) & blackPawns;
		movers |= ((tmp & MASK_D5) << 5) & blackPawns;
	}
	
	// open back (3|5) == black; then  back 4 == white;
	// then open is a space to jump to;
	
	tmp  =  ((openSquares & MASK_D3) << 3) & whitePawns;
	tmp |=  ((openSquares & MASK_D5) << 5) & whitePawns;
	movers |= (tmp << 4) & blackPawns;
	
	if (blackKings){
		tmp = (openSquares >> 4) & whitePawns;
		if (tmp){
			movers |= ((tmp&MASK_U3) >> 3) & blackKings;
			movers |= ((tmp&MASK_U5) >> 5) & blackKings;
		}
		tmp =  ((openSquares&MASK_U3) >> 3) & whitePawns;
		tmp |= ((openSquares&MASK_U5) >> 5) & whitePawns;
		if (tmp){
			movers |= (tmp >> 4) & blackKings;
		}
	}
	return movers;
}
*/

// should be correct, but need to test
ULONG moveGenerator::sliders( player_t player){
	const ULONG openSquares = ~(main_Board.whitePawns | main_Board.blackPawns); // Not Occupied
 	ULONG One, Two;
 	ULONG movers = 0;
	
	// replace with XOR
	
	if(player == WHITE){
		One = main_Board.whitePawns;
		Two	= main_Board.whitePawns&main_Board.kings;
	}else{
		One = main_Board.blackPawns&main_Board.kings;
		Two = main_Board.blackPawns;
	}
	
	if (One){
		movers |= (openSquares >> 4) & One;
		movers |= ((openSquares & MASK_D5) >> 5) & One;
		movers |= ((openSquares & MASK_D3) >> 3) & One;
	}
	if (Two) {
		movers |= (openSquares << 4) & Two;
		movers |= ((openSquares & MASK_U3) << 3) & Two;
		movers |= ((openSquares & MASK_U5) << 5) & Two;
	}
	return movers;
}
ULONG moveGenerator::jumpers( player_t player){	// true == white to move
	const ULONG openSquares = ~(main_Board.blackPawns | main_Board.whitePawns);
 	ULONG movers = 0;
 	ULONG tmp = 0;

	
	// replace with XOR
	
 	ULONG One, Two, Three;	
	if(player == WHITE){
		One = main_Board.blackPawns;
		Two = main_Board.whitePawns;
		Three = main_Board.whitePawns & main_Board.kings;
	}else{
		One = main_Board.whitePawns;
		Two = main_Board.blackPawns & main_Board.kings;
		Three = main_Board.blackPawns;
	}
	
	if(Two){
		tmp = (openSquares >> 4) & One;
		if (tmp){
			movers |= ((tmp & MASK_D3) >> 3) & Two;
			movers |= ((tmp & MASK_D5) >> 5) & Two;
		}
		
		tmp  =  ((openSquares & MASK_D3) >> 3) & One;
		tmp |=  ((openSquares & MASK_D5) >> 5) & One;
		movers |= (tmp >> 4) & Two;
	}
	if (Three){
		tmp = (openSquares<< 4) & One;
		if (tmp){
			movers |= ((tmp&MASK_U3) << 3) & Three;
			movers |= ((tmp&MASK_U5) << 5) & Three;
		}
		tmp =  ((openSquares&MASK_U3) << 3) & One;
		tmp |= ((openSquares&MASK_U5) << 5) & One;
		if (tmp){
			movers |= (tmp << 4) & Three;
		}
	}
	
	return movers;
}

void moveGenerator::nextBoard(){
	

	if (!current_peice){
		current_Board = board();
		return;
	}
	
	
	int dir = (player==WHITE ? +1 : -1 );	// white=1,	black=0
											// king=1,	!king=0
											// logical XOR
	
	current_Board = main_Board;
	
	
	
	
	/*
	 
	test << 4*(White | BlackKing)
		valid_slide = ((current_peice & (current_Board.whitePawns | (current_Board.blackPawns & current_Board.kings))) << 4) & ~(my_peices | your_peices)
 
 
	test << (3|5)*(White | BlackKing)	 
		valid_slide = ((current_peice & MASK_D3 & (current_Board.blackPawns | (current_Board.whitePawns & current_Board.kings))) >> 3) & ~(my_peices | your_peices) & ()
		valid_slide = ((current_peice & MASK_D5 & (current_Board.blackPawns | (current_Board.whitePawns & current_Board.kings))) >> 5) & ~(my_peices | your_peices) & ()

		
	test >> 4*(Black | WhiteKing)
		valid_slide = ((current_peice & (current_Board.blackPawns | (current_Board.whitePawns & current_Board.kings))) >> 4) & ~(my_peices | your_peices)

 
	test >> (3|5)*(Black | WhiteKing)
	 
	*/
	
	
	
	
	if (jumping_peices){
		// nop();
	}else{ // sliding_peices
		ULONG testsquare;
//		ULONG testsquare = (current_peice << (dir*4)) & ~(my_peices | your_peices);
		
		if ( testsquare = ((player==WHITE)*(current_peice << 4) | (player==BLACK)*(current_peice >> 4)) & ~(my_peices | your_peices) ){ // +4

			if (player == WHITE){
				current_Board.whitePawns &= ~current_peice;
				current_Board.whitePawns |= testsquare;
			}else{
				current_Board.blackPawns &= ~current_peice;
				current_Board.blackPawns |= testsquare;				
			}
			
			if (current_peice & current_Board.kings){
				current_Board.kings &= ~current_peice;
				current_Board.kings |= testsquare;
			}
			
			
		}else{
			
			cout << "Error: peice (" << current_peice << ") with no move" << endl;
		}

	}
	
	
	do{
		current_peice = (current_peice << 1);
	}while (current_peice && !(current_peice & my_peices));
	
	
	
	
	
 }

board moveGenerator::curBoard(){
	return current_Board;
}

board & moveGenerator::operator*(){
	return this->current_Board;
}
moveGenerator& moveGenerator::operator++(){		// ++x;
	nextBoard();
	return *this;
}
moveGenerator moveGenerator::operator++(int){	// x++
	moveGenerator mg = *this;
	nextBoard();
	return mg;
}


/*
Max(board, a,b){
	if (terminal(board))
		retrn v;
	
	v = -inf; // below minamum BEF 
	for each move m from board {
		v = maximum(v, Min(board(m), a, b));
	}
	
	if (v >= b )//prune/
		return v;
	
	a = maximum(a, v);
}

		
Min(board, a,b){
	if (terminal(board))
		retrn v;
	
	v = inf; // above maximum BEF 
	for each move m from board {
		v = minimum(v, Max(board(m), a, b));
	}
	
	if (v <= a )//prune
		return v;
	
	b = minimum(b, v);
}
		
terminal(){
	- at max depth
	- end game database search
	- cache
}
		
Ab_search(board){
	v = max(board, -inf, inf);
	return v;
}
*/
