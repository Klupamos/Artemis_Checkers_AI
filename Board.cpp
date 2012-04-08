/*
 *  Board.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/16/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */


#include <iostream>
using std::cout; using std::endl;
#include <string>

#include "Board.h"


void board::printBoard() const{	
	ULONG WK = kings & whitePawns;
	ULONG WP = whitePawns;
	ULONG BK = kings & blackPawns;
	ULONG BP = blackPawns;
	
	for(ULONG i=0x80000000; i>0; i = i>>1){
		
		if (i & 0x80808080)
			cout << "  ";
		
		if( WK & i){
			cout << "WK";
		}else if( WP & i){
			cout << "WW";
		}else if( BK & i){
			cout << "BK";
		}else if( BP & i){
			cout << "BB";
		}else{
			cout << "--";
		}
		cout << "  ";
		
		if (i & 0x11111111)
			cout << endl;
	}
	cout << endl << endl;
}

std::ostream & operator<<(std::ostream & theStream, const board & table){	
	ULONG WK = table.kings & table.whitePawns;
	ULONG WP = table.whitePawns;
	ULONG BK = table.kings & table.blackPawns;
	ULONG BP = table.blackPawns;
	
	for(ULONG i=0x80000000; i>0; i = i>>1){
		
		if (i & 0x80808080)
			theStream << "  ";
		
		if( WK & i){
			theStream << "WK";
		}else if( WP & i){
			theStream << "WW";
		}else if( BK & i){
			theStream << "BK";
		}else if( BP & i){
			theStream << "BB";
		}else{
			theStream << "--";
		}
		theStream << "  ";
		
		if (i & 0x11111111)
			theStream << endl;
	}
	theStream << endl << endl;
	
	return theStream;
}

bool board::winner() const{
	return (whitePawns == 0 || blackPawns == 0);
}

std::string board::toSeq() const{
	
	ULONG WK = kings & whitePawns;
	ULONG WP = whitePawns;
	ULONG BK = kings & blackPawns;
	ULONG BP = blackPawns;
	std::string seq;
	
	for(ULONG bit=1; bit>0; bit = bit << 1){
		
		if(bit & WK){
			seq.append(1, 'W');
			continue;
		}
		if(bit & WP){
			seq.append(1, 'w');
			continue;
		}
		if(bit & BK){
			seq.append(1, 'B');
			continue;
		}
		if(bit & BP){
			seq.append(1, 'b');
			continue;
		}
		
		seq.append(1, '_');
		continue;
	}
	return seq;
}

void board::fromSeq(const std::string & seq){
	int pos=0;
	ULONG bit=1;
	kings = whitePawns = blackPawns = 0;
	while(pos<32){		
		switch (seq[pos]) {
			
			case 'B':
				kings |= bit;
			case 'b':
				blackPawns |= bit;
				break;

				
			case 'W':
				kings |= bit;
			case 'w':
				whitePawns |= bit;
				break;

		}
		++pos;
		bit = bit<<1;
	}
}