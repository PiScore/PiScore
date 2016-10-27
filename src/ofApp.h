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

#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define BUILD 0
// Windows:      0
// MacOS:        1
// Raspberry Pi: 2

class ofApp : public ofBaseApp {

public:
	string appVersionID;
	string build;

	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	ofImage clefs;
	ofSoundPlayer audio;
	int scoreWidth;
	int scoreHeight;

	int mouseX, mouseY, pMouseX, pMouseY, mouseXScreen, mouseYScreen;

	int penSize;
	ofColor penColor;

	float scoreFitToHeight;

	bool editModep;
	bool eraserp;

	const int playheadPos = (ofGetWidth() / 5);

	const int notAvailableAlpha = 20;
	const int semiTransparent = 178;

	int appState;
	int wizardStep;

	int elapsedTimeMS;

	const float fps = 60;

	string tempTextBuffer;
	string wizardText[7];

	int start;
	int end;
	int clefsStart;
	int clefsEnd;
	//int adjStart, adjEnd;
	float dur;
	float preroll;
	float zoom;
	int vOffset;
	int totalPx;
	int totalFrames;
	int frameCounter;
	int scoreX;
	int localScoreX;
	int audioStart; // Which ms value in the audio file should line up with (score) start
	int audioMS;

	bool playingp;

	string pathAudio;
	bool audiop;

	string pathServerp;
	ofFile fileServerp;
	bool serverp;

	ofFile fileScreenSettings;
	string pathScreenSettings;
	bool screenSettingsp;
	int screenSettingsStep;
	vector<string> loadedScreenSettingsData;

	vector <ofImage> scoreTilesImages;
	vector <ofImage> canvasTilesImages;
	int scoreTotalWidth;
	int scoreTotalHeight;

	bool raspiScorePathDialogp;
	vector <char> tmpDisplayString;

	vector<string> license;
	ofBuffer licenseBuffer;
	bool viewLicensep;

	int width;
	int tempWidth;
	int height;
	int tempHeight;

	ofFile fileBroadcastIP;
	string pathBroadcastIP;
	bool ipEditp;
	int broadcastIpStep;
	vector<string> loadedBroadcastIPData;
	string broadcastIp;
	int broadcastPort;
	string tempBroadcastIp;
	int tempBroadcastPort;

	string pathPrevScore;
	ofFile filePrevScore;

	string pathCanvas;

	string pathLoadedScore;
	string pathLoadedScoreConf;
	ofFile fileLoadedScoreConf;
	bool loadedScorep;
	bool loadedScoreConfp;

	bool mousePressedp;

	string userPath;

	string scorePath;
	vector <string> scorePathVector;
	vector <string> canvasPathVector;
	string scoreConfPath;
	string audioPath;
	vector <string> scoreConfVector;

	// FUNCTIONS
	int calculateScoreX(int);
	int calculateScoreXFromAudio(int);
	string getStringFromPath(string path);
	vector <string> scorePathLoader(string path);
	vector <string> canvasPathLoader(vector <string> scoreVector);
	string scoreConfPathLoader(string scorePath);
	string audioPathLoader(string scorePath);
	vector <string> getScoreConfs(string scoreConfPath);

	// GUI
	string iconPath;
	int iconSize;
	int iconSizeLarge;
	int iconSizeHuge;
	int iconPadding;
	ofImage piscoreIcon;
	ofImage editButton;
	ofImage resetButton;
	ofImage playButton;
	ofImage pauseButton;
	ofImage rewindButton;
	ofImage fastForwardButton;
	ofImage zoomButton;
	ofImage shutdownButton;
	ofImage homeButton;
	ofImage zeroButton;
	ofImage plusButton;
	ofImage minusButton;
	ofImage upButton;
	ofImage downButton;
	ofImage leftButton;
	ofImage cancelButton;
	ofImage confirmButton;
	ofImage launchButton;
	ofImage loadButton;
	ofImage wizardButton;
	ofImage emptyButton;
	ofImage checkedButton;
	ofImage screenButton;
	ofImage pencilWritingButton;
	ofImage eraserButton;

	ofxOscSender sender;
	ofxOscReceiver receiver;

	int dialogTimeout;
	int zoomDialogSchedule;
	bool zoomDialogp;
	int exitDialogSchedule;
	bool exitDialogp;
};
