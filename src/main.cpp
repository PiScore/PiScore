//PiScore - a system for playing back scrolling musical scores
//and keeping in sync across multiple devices.
//Copyright (C) 2016  David Stephen Grant
//
//This file is part of PiScore.
//
//PiScore is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//PiScore is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with PiScore.  If not, see <http://www.gnu.org/licenses/>.

#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
	//FreeConsole();                              // <-------- Hide console

	ofSetupOpenGL(1024,768,OF_WINDOW);			// <-------- setup the GL context
	ofHideCursor();
	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());
	
}
