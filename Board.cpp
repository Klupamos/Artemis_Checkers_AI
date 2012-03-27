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

#include "Board.h"


void board::printBoard() const{
	//cout << "Board" << endl;
	
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

bool board::winner() const{
	return (whitePawns == 0 || blackPawns == 0);
}
