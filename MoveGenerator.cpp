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

#include "MoveGenerator.h"
#include "Color.h"

moveGenerator::moveGenerator(color_t p = WHITE, board newBoard = board()):player(p), main_Board(newBoard), stack_pos(0){
	current_Piece[0] = 1;
	current_Board[0] = newBoard;
	next_Test[0] = F4;
	
	sliding_pieces = sliders(player);
	jumping_pieces = jumpers(player);

	my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[0].whitePawns))) | (player == BLACK) * long(&(current_Board[0].blackPawns)));
	your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[0].blackPawns))) | (player == BLACK) * long(&(current_Board[0].whitePawns)));

	
	sliding_pieces = sliders(player);
	jumping_pieces = jumpers(player);	
	
	
	my_movable_pieces = *my_pieces & ((jumping_pieces) | (sliding_pieces) * !bool(jumping_pieces));
		
	
	while (current_Piece[0] && !(current_Piece[0] & my_movable_pieces)){
		current_Piece[0] = (current_Piece[0] << 1);
	}
	
	
	nextBoard(); //set first move
}

// rewrite these to take no inputs
ULONG moveGenerator::sliders(color_t player){
	const ULONG openSquares = ~(main_Board.whitePawns | main_Board.blackPawns); // Not Occupied
 	ULONG One, Two;
 	ULONG movers = 0;
		
	if(player == WHITE){
		One = main_Board.whitePawns;
		Two	= main_Board.whitePawns&main_Board.kings;
	}else{
		One = main_Board.blackPawns&main_Board.kings;
		Two = main_Board.blackPawns;
	}
	
	if (One){
		movers |= One & (openSquares >> 4);
		movers |= One & ((openSquares & MASK_B5) >> 5); // open & back5 == One and forward5 
		movers |= One & ((openSquares & MASK_B3) >> 3); 
	}
	if (Two) {
		movers |= Two & (openSquares << 4);
		movers |= Two & ((openSquares & MASK_F5) << 5); // open & forward5 == Two and back5 
		movers |= Two & ((openSquares & MASK_F3) << 3); 
	}
	return movers;
}
ULONG moveGenerator::jumpers(color_t player){	// true == white to move
	const ULONG openSquares = ~(main_Board.blackPawns | main_Board.whitePawns);
 	ULONG movers = 0;
 	ULONG tmp = 0;
	
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
		movers |= ((tmp & MASK_B5) >> 5) & Two;
		movers |= ((tmp & MASK_B3) >> 3) & Two;
		
		tmp  =  ((openSquares & MASK_B3) >> 3) & One;
		tmp |=  ((openSquares & MASK_B5) >> 5) & One;
		movers |= (tmp >> 4) & Two;
	}
	
	if (Three){
		tmp = (openSquares << 4) & One;
		movers |= ((tmp & MASK_F3) << 3) & Three;
		movers |= ((tmp & MASK_F5) << 5) & Three;
		
		tmp =  ((openSquares & MASK_F3) << 3) & One;
		tmp |= ((openSquares & MASK_F5) << 5) & One;
		movers |= (tmp << 4) & Three;
	}
	
	return movers;
}

void moveGenerator::nextBoard(){
	
	
	current_Board[0] = main_Board;
	
	ULONG mid_space = 0;
	ULONG end_space = 0;
	
	if (jumping_pieces){
		
		
		if (stack_pos > 0){
//			cout << "Poping Back (1): " << (int)stack_pos << " : " << next_Test[stack_pos] << endl;

			
			stack_pos--;
			my_pieces   = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
			your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));
			
			
			switch (next_Test[stack_pos]){
				case F35:
					goto Test_F35;
				case B4:
					goto Test_B4;
				case B35:
					goto Test_B35;
				case INC:
					goto Ret;
			}
		}
				
		while (true){
			end_of_chain[stack_pos] = true;
			
//			cout << "Testing F4";
			if (
				(mid_space = ((current_Piece[stack_pos] & (current_Board[stack_pos].whitePawns | (current_Board[stack_pos].blackPawns & current_Board[stack_pos].kings))) << 4) & *your_pieces)
				&&
				(
				 (mid_space & MASK_F3) &&
				 (end_space = (mid_space << 3) & ~(*my_pieces | *your_pieces))
				 ||
				 (mid_space & MASK_F5) && 
				 (end_space = (mid_space << 5) & ~(*my_pieces | *your_pieces))
				)
				){
//				cout << ": 1" << endl;
				end_of_chain[stack_pos] = false;
				
				
				next_Test[stack_pos] = F35;								// store where to call next once im back
				current_Board[stack_pos+1] = current_Board[stack_pos];	// store what im coming back to 
				stack_pos++;
				
				current_Piece[stack_pos] = end_space;
				
				// set up what i will be working on
				my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
				your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));

				
				// place end_piece
				*my_pieces |= end_space;
				current_Board[stack_pos].kings |= bool(current_Board[stack_pos].kings & current_Piece[stack_pos-1]) * end_space;
				
				// remove start_piece
				*my_pieces &= ~current_Piece[stack_pos-1];
				current_Board[stack_pos].kings &= ~current_Piece[stack_pos-1];
				
				// remove jumped_piece
				*your_pieces &= ~mid_space;
				current_Board[stack_pos].kings &= ~mid_space;
				
				continue;
			}
//			cout << ": 0" << endl;
			
		Test_F35:
//			cout << "Testing F35\n";
			//cout << "\t" << (mid_space << 4) << "\t";
			if (
				(
				 (current_Piece[stack_pos] & MASK_F3 & (current_Board[stack_pos].whitePawns | (current_Board[stack_pos].blackPawns & current_Board[stack_pos].kings)) ) &&
				 (mid_space = (current_Piece[stack_pos] << 3) & *your_pieces)
				 ||
				 (current_Piece[stack_pos] & MASK_F5 & (current_Board[stack_pos].whitePawns | (current_Board[stack_pos].blackPawns & current_Board[stack_pos].kings)) ) && 
				 (mid_space = (current_Piece[stack_pos] << 5) & *your_pieces)
				)
				&&
				(end_space = (mid_space << 4) & ~(*my_pieces | *your_pieces))
				){
//				cout << ": 1" << endl;
				end_of_chain[stack_pos] = false;
				
				next_Test[stack_pos] = B4;								// store where to call next once im back
				current_Board[stack_pos+1] = current_Board[stack_pos];	// store what im coming back to 
				stack_pos++;
				
				current_Piece[stack_pos] = end_space;
				
				// set up what i will be working on
				my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
				your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));
				
				
				// place end_piece
				*my_pieces |= end_space;
				current_Board[stack_pos].kings |= bool(current_Board[stack_pos].kings & current_Piece[stack_pos-1]) * end_space;
				
				// remove start_piece
				*my_pieces &= ~current_Piece[stack_pos-1];
				current_Board[stack_pos].kings &= ~current_Piece[stack_pos-1];
				
				// remove jumped_piece
				*your_pieces &= ~mid_space;
				current_Board[stack_pos].kings &= ~mid_space;
				
				continue;
			}
//			cout << ": 0" << endl;
			
			Test_B4:
//			cout << "Testing B4";
			if (
				(mid_space = ((current_Piece[stack_pos] & (current_Board[stack_pos].blackPawns | (current_Board[stack_pos].whitePawns & current_Board[stack_pos].kings))) >> 4) & *your_pieces)
				&&
				(
				 (mid_space & MASK_B3) &&
				 (end_space = (mid_space >> 3) & ~(*my_pieces | *your_pieces))
				 ||
				 (mid_space & MASK_B5) && 
				 (end_space = (mid_space >> 5) & ~(*my_pieces | *your_pieces))
				 )
				
				){
//				cout << ": 1" << endl;
				end_of_chain[stack_pos] = false;
				
				
				next_Test[stack_pos] = B35;								// store where to call next once im back
				current_Board[stack_pos+1] = current_Board[stack_pos];	// store what im coming back to 
				stack_pos++;
				
				current_Piece[stack_pos] = end_space;
				
				// set up what i will be working on
				my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
				your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));
				
				
				// place end_piece
				*my_pieces |= end_space;
				current_Board[stack_pos].kings |= bool(current_Board[stack_pos].kings & current_Piece[stack_pos-1]) * end_space;
				
				// remove start_piece
				*my_pieces &= ~current_Piece[stack_pos-1];
				current_Board[stack_pos].kings &= ~current_Piece[stack_pos-1];
				
				// remove jumped_piece
				*your_pieces &= ~mid_space;
				current_Board[stack_pos].kings &= ~mid_space;
				
				continue;
			}
//			cout << ": 0" << endl;
			
			Test_B35:
//			cout << "Testing B35";
			if (
				(
				 (current_Piece[stack_pos] & MASK_B3 & (current_Board[stack_pos].blackPawns | (current_Board[stack_pos].whitePawns & current_Board[stack_pos].kings)) ) &&
				 (mid_space = (current_Piece[stack_pos] >> 3) & *your_pieces)
				 ||
				 (current_Piece[stack_pos] & MASK_B5 & (current_Board[stack_pos].blackPawns | (current_Board[stack_pos].whitePawns & current_Board[stack_pos].kings)) ) && 
				 (mid_space = (current_Piece[stack_pos] >> 5) & *your_pieces)
				 )
				&&
				(end_space = (mid_space >> 4) & ~(*my_pieces | *your_pieces))
				
				){
//				cout << ": 1" << endl;
				end_of_chain[stack_pos] = false;
				
				next_Test[stack_pos] = INC;								// store where to call next once im back
				current_Board[stack_pos+1] = current_Board[stack_pos];	// store what im coming back to 
				stack_pos++;
				
				current_Piece[stack_pos] = end_space;
				
				// set up what i will be working on
				my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
				your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));
				
				
				// place end_piece
				*my_pieces |= end_space;
				current_Board[stack_pos].kings |= bool(current_Board[stack_pos].kings & current_Piece[stack_pos-1]) * end_space;
				
				// remove start_piece
				*my_pieces &= ~current_Piece[stack_pos-1];
				current_Board[stack_pos].kings &= ~current_Piece[stack_pos-1];
				
				// remove jumped_piece
				*your_pieces &= ~mid_space;
				current_Board[stack_pos].kings &= ~mid_space;
				
				continue;
			}
//			cout << ": 0" << endl;
			
			Ret:
			if(stack_pos <= 0){
				stack_pos = 1; //Pop Back(2) will decrease to 0;
				end_of_chain[1] = false;
				next_Test[0] = F4;
//				cout << "Inc Piece" << endl;
				do{
					current_Piece[0] = (current_Piece[0] << 1);
				}while (current_Piece[0] & ~my_movable_pieces);
				
				if (!current_Piece[0]){
					current_Board[0] = board();
					return;
				}
			}
			
			
			if (end_of_chain[stack_pos]){
//				cout << "End of Chain" << endl;
				current_Board[0] = current_Board[stack_pos];
				
				current_Board[0].kings |= (current_Board[0].whitePawns & 0xF0000000);
				current_Board[0].kings |= (current_Board[0].blackPawns & 0x0000000F);
				return;
			}
			
			
			//pop back again
//			cout << "Poping Back (2): " << (int)stack_pos << " : " << next_Test[stack_pos] << endl;
			
			stack_pos--;
			my_pieces =   (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].whitePawns))) | (player == BLACK) * long(&(current_Board[stack_pos].blackPawns)));
			your_pieces = (ULONG*)(((player == WHITE) * long(&(current_Board[stack_pos].blackPawns))) | (player == BLACK) * long(&(current_Board[stack_pos].whitePawns)));
			
			
			switch (next_Test[stack_pos]){
				case F35:
					goto Test_F35;
				case B4:
					goto Test_B4;
				case B35:
					goto Test_B35;
				case INC:
					goto Ret;
			}
			
		}
		
	}else{ // sliding_pieces
		
		switch (next_Test[0]) {
			case INC:
//				cout << "Increment and Continue" << endl;
				do{
					current_Piece[0] = (current_Piece[0] << 1);
				}while (current_Piece[0] & ~my_movable_pieces);
				
				if (!current_Piece[0]){
					current_Board[0] = board();
					return;
				}
			next_slider_F4:
			case F4:
//				cout << "Testing F4" << endl;
				//if ( end_space = ((player==WHITE)*(current_Piece << 4) | (player==BLACK)*(current_Piece >> 4)) & ~(my_pieces | your_pieces) ){ // +4
				if ( end_space = ((current_Piece[0] & (current_Board[0].whitePawns | (current_Board[0].blackPawns & current_Board[0].kings))) << 4) & ~(*my_pieces | *your_pieces) ){
					next_Test[0] = F35;
					break;
				}
			case F35:
//				cout << "Testing F35" << endl;
								
				if ( 
					(current_Piece[0] & MASK_F5)  && 
					(end_space = ((current_Piece[0] & (current_Board[0].whitePawns | (current_Board[0].blackPawns & current_Board[0].kings))) << 5) & ~(*my_pieces | *your_pieces))
					
					||
					
					(current_Piece[0] & MASK_F3)  && 
					(end_space = ((current_Piece[0] & (current_Board[0].whitePawns | (current_Board[0].blackPawns & current_Board[0].kings))) << 3) & ~(*my_pieces | *your_pieces))
					){
					
					
					next_Test[0] = B35;
					break;
				}
			
			case B35:
//				cout << "Testing B35" << endl;
				if ( 
					(current_Piece[0] & MASK_B5)
					&& 
					(end_space = ((current_Piece[0] & (current_Board[0].blackPawns | (current_Board[0].whitePawns & current_Board[0].kings))) >> 5) & ~(*my_pieces | *your_pieces))
					
					||
					
					(current_Piece[0] & MASK_B3)
					&& 
					(end_space = ((current_Piece[0] & (current_Board[0].blackPawns | (current_Board[0].whitePawns & current_Board[0].kings))) >> 3) & ~(*my_pieces | *your_pieces))
					){
					next_Test[0] = B4;
					break;
				}
				
			case B4:
//				cout << "Testing B4" << endl;
				if ( end_space = ((current_Piece[0] & (current_Board[0].blackPawns | (current_Board[0].whitePawns & current_Board[0].kings))) >> 4) & ~(*my_pieces | *your_pieces) ){
					next_Test[0] = INC;
					break;
				}
				
				
//				cout << "Reset and Increment" << endl;
				do{
					current_Piece[0] = (current_Piece[0] << 1);
				}while (current_Piece[0] & ~my_movable_pieces);
				
				if (!current_Piece[0]){
					current_Board[0] = board();
					return;
				}
				
				goto next_slider_F4;
				
				break;
				
				
			default:
				cout << "Error: piece (" << current_Piece[0] << ") with no move" << endl;
				current_Board[0] = board();
				return;
		}
		
	}
	
	// removed any jumped piece
	*my_pieces &= ~current_Piece[0];
//	*your_pieces &= ~mid_space; // only for jumps
	*my_pieces |= end_space;
	
	// move king
//	current_Board[0].kings &= ~mid_space; // only for jumps
	if (current_Board[0].kings & current_Piece[0]){
		current_Board[0].kings &= ~current_Piece[0];
		current_Board[0].kings |= end_space;
	}

	// moved piece becomes king?
	current_Board[0].kings |= (current_Board[0].whitePawns & 0xF0000000);
	current_Board[0].kings |= (current_Board[0].blackPawns & 0x0000000F);
	
 }

board moveGenerator::curBoard(){
	return current_Board[0];
}

board & moveGenerator::operator*(){
	return this->current_Board[0];
}

board* moveGenerator::operator->(){
	return &(this->current_Board[0]);
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
		retrn v = FFNN(board);
	
	v = -inf; // below minamum BEF 
	for each move m from board {
		v = maximum(v, Min(board(m), a, b));
	}
	
	if (v >= b)//prune/
		return v;
	
	a = maximum(a, v);
}

		
Min(board, a,b){
 
	if (terminal(board))
		retrn v = FFNN(board);
	
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
 
	local_board = mg.current();

	Max(board, inf, -inf);
	
}
*/
