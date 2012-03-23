/*
 *  Person.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/18/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include "Piece.h"

piece_t operator!(const piece_t & p){
	if (p == WHITE){
		return BLACK;
	}else{
		return WHITE;
	}
};
