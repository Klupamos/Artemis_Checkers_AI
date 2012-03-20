/*
 *  Person.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/18/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include "Person.h"

player_t operator!(const player_t & p){
	if (p == WHITE){
		return BLACK;
	}else{
		return WHITE;
	}
};
