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

enum piece_t {WHITE, BLACK};

piece_t operator!(const piece_t & p);

#endif //FILE_PIECE_H_INCLUDED