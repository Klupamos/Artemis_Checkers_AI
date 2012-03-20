/*
 *  Person.h
 *  artemis
 *
 *  Created by Gregory Klupar on 3/18/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_PERSON_H_INCLUDED
#define FILE_PERSON_H_INCLUDED

enum player_t {WHITE, BLACK};

player_t operator!(const player_t & p);

#endif //FILE_PERSON_H_INCLUDED