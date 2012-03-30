/*
 *  Person.cpp
 *  artemis
 *
 *  Created by Gregory Klupar on 3/18/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include "Color.h"

color_t operator!(const color_t & p){
	if (p == WHITE){
		return BLACK;
	}else{
		return WHITE;
	}
};
