/*
 *  GUI.h
 *  Artemis
 *
 *  Created by Gregory Klupar on 3/29/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#ifndef FILE_GUI_H_INCLUDED
#define FILE_GUI_H_INCLUDED 

struct interface{
	Player mrBlack;
	Player mrWhite;
	
	board officialBoard;
	
}


void drawBoard();
void drawMenu();

// glew functions
void renderDisplay();
void KeboardInterupt();


#endif