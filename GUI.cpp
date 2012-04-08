/*
 *  GUI.cpp
 *  Artemis
 *
 *  Created by Gregory Klupar on 3/29/12.
 *  Copyright 2012 Newline Productions. All rights reserved.
 *
 */

#include "GUI.h"

#ifndef __APPLE__
# include <GL/glut.h>    // GLUT stuff, includes OpenGL headers as well
#else
# include <GLUT/glut.h>  // Apple puts glut.h in a different place
# include <OpenGL/glext.h>
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
#endif

interface GUI;	// the GUI global

interface::interface(){
	MrWhite = Player(BLACK);
}


void GUI_init(int argc, char ** argv){
	//global
	GUI.menu_displayed = true;
	
	
	// Initialize OpenGL/GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	
    // Make a window
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("Artemis GUI");
	
    // Initialize GL states & register GLUT callbacks
    glClearColor(0.4, 0.4, 0.4, 0.0);
	
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1., 1., -1., 1.);
	
    glMatrixMode(GL_MODELVIEW);  // Always go back to model/view mode 
	
	
	glutDisplayFunc(renderDisplay); 
    glutIdleFunc(idle);
    glutKeyboardFunc(keboardInput);
	
    // Do something
    glutMainLoop();
}

void renderDisplay(){
	if(GUI.menu_displayed == true){
		drawMenu();
	}else{
		drawBoard();
	}
}

void drawMenu(){ //draw 100 labeled boxes
	
}

void drawBoard(){
	
}