/*
 *  Person.h
 *  artemis
 *
 *  Created by Gregory Klupar on 3/18/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_PIECE_H_INCLUDED
#define FILE_PIECE_H_INCLUDED

enum color_t {WHITE, BLACK};

color_t operator!(const color_t & p);

#endif //FILE_PIECE_H_INCLUDED