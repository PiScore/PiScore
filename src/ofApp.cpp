#include "ofApp.h"
#include <fstream>
#include <stdlib.h>
//#include <chrono>
//#include <thread>

//--------------------------------------------------------------
void ofApp::setup() {
        appVersionID = "0.3.2";
		build = "windows"; // "windows", "osx", "raspi"

		if (build == "windows") {
			userPath = getenv("USERPROFILE"); // Windows build
		}
		else if (build == "osx") {
			userPath = getenv("HOME");
			ofSetDataPathRoot("../Resources/data/");
			// In Xcode project add following line to "Build Phases" --> "Run Script",
			// making sure the preceding line ends with a semi-colon:
			// cp -r bin/data "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/Resources";
			// Set icon path in Project.xccode to bin/data/gui/ !!without string quotes!!
			// and icon names to PiScore.icns
		}
		else if (build == "raspi") {
			userPath = getenv("HOME");
		}

	ofSetWindowTitle("PiScore v" + appVersionID);

	pathScreenSettings = ofFilePath::getAbsolutePath(userPath + "/.piscore/screensettings");
	if (!ofFile::doesFileExist(pathScreenSettings)) {
		width = 800;
		height = 480;
	}
	else {
		ifstream fin; //declare a file stream  
		fin.open(pathScreenSettings.c_str()); //open your text file
		for (int i = 0; i < 2; i++) {
			string str; //declare a string for storage  
			getline(fin, str); //get a line from the file, put it in the string  
			loadedScreenSettingsData.push_back(str);
		}
		fin.close();
		width = atoi(loadedScreenSettingsData[0].c_str());
		height = atoi(loadedScreenSettingsData[1].c_str());
	}

	ofSetWindowShape(width, height);

	penSize = 2;
	penColor = ofColor(0, 255);
	editModep = false;
	eraserp = false;

	mousePressedp = false;

	//ofSetWindowPosition(0, 0);
	ofSetFrameRate(fps);
	ofSetEscapeQuitsApp(false);
	ofSetFullscreen(false);
	ofBackground(ofColor::white);

	licenseBuffer = ofBufferFromFile(ofFilePath::getAbsolutePath("license/LICENSE-SHORT"));
	for (auto line : licenseBuffer.getLines()) {
		license.push_back(line);
	}
	for (int i = 0; i < license.size(); i++) {
		/* Print License to Console */ cout << license[i] << endl;
	}
	/* License formatting */ cout << endl << endl;
	viewLicensep = false;

	appState = 0;

	ipEditp = false;
	screenSettingsp = false;

	wizardText[0] = "Where does the music start?";
	wizardText[1] = "Where does the music end?";
	wizardText[2] = "Where do the clefs start?";
	wizardText[3] = "Where do the clefs end?";
	wizardText[4] = "Enter the duration (seconds)";
	wizardText[5] = "Enter the preroll duration (seconds)";
	wizardText[6] = "Enter the audio sync. point (ms)";

	pathPrevScore = ofFilePath::getAbsolutePath(userPath + "/.piscore/prevscore");
	if (!ofFile::doesFileExist(pathPrevScore)) {
		loadedScorep = false;
		loadedScoreConfp = false;
		audiop = false;
	}
	else {
		ifstream fin;
		string str;
		fin.open(pathPrevScore.c_str());
		getline(fin, str);
		if (ofFile::doesFileExist(str)) {
			string ext = ofFilePath::getFileExt(str);
			ofDirectory dir(ofFilePath::getEnclosingDirectory(str));
			//load show files with same extension
			dir.allowExt(ext);
			//populate the directory object
			dir.listDir();
			for (int i = 0; i < dir.size(); i++) {
				string tmpPath = dir.getPath(i);
				string tmpCanvasPath = ofFilePath::getEnclosingDirectory(tmpPath) + "annotations/" + ofFilePath::getBaseName(tmpPath) + "-annotations.png";
				if (tmpPath.find("-annotations.png") == std::string::npos) { // Only load if -annotations.png is not part of the filename
					scoreTilesPaths.push_back(tmpPath);
					canvasTilesPaths.push_back(tmpCanvasPath);
				}
			}

			loadedScorep = true;
			filePrevScore.open(pathPrevScore, ofFile::WriteOnly);
			filePrevScore << str << endl;
			filePrevScore.close();
			pathLoadedScore = str;
			if (build != "raspi") {
				pathAudio = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + ".wav";
				//pathCanvas = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + "-canvas.png";
				if (ofFile::doesFileExist(pathAudio)) {
					audiop = true;
				}
				else audiop = false;
			}
			pathLoadedScoreConf = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + ".piscore";
			if (ofFile::doesFileExist(pathLoadedScoreConf)) {
				loadedScoreConfp = true;
				ifstream fin; //declare a file stream  
				fin.open(pathLoadedScoreConf.c_str()); //open your text file
				for (int i = 0; i < 9; i++) {
					string str; //declare a string for storage  
					getline(fin, str); //get a line from the file, put it in the string  
					loadedScoreConfData.push_back(str);
				}
				fin.close();
				start = atoi(loadedScoreConfData[0].c_str());
				end = atoi(loadedScoreConfData[1].c_str());
				clefsStart = atoi(loadedScoreConfData[2].c_str());
				clefsEnd = atoi(loadedScoreConfData[3].c_str());
				dur = atof(loadedScoreConfData[4].c_str());
				preroll = atof(loadedScoreConfData[5].c_str());
				zoom = atof(loadedScoreConfData[6].c_str());
				vOffset = atoi(loadedScoreConfData[7].c_str());
				audioStart = atoi(loadedScoreConfData[8].c_str());
			}
			else {
				loadedScoreConfp = false;
			}
		}
		else {
			loadedScorep = false;
			loadedScoreConfp = false;
			audiop = false;
		}
		fin.close();
	}

	scoreX = start; // init value if !serverp has not yet received any messages
	localScoreX = scoreX;

	pathServerp = ofFilePath::getAbsolutePath(userPath + "/.piscore/serverp");
	if (!ofFile::doesFileExist(pathServerp)) {
		serverp = false;
	}
	else {
		ifstream fin;
		string str;
		fin.open(pathServerp.c_str());
		getline(fin, str);
		if (str == "1") serverp = true;
		else serverp = false;
		fin.close();
	}

	pathBroadcastIP = ofFilePath::getAbsolutePath(userPath + "/.piscore/broadcastip");
	if (!ofFile::doesFileExist(pathBroadcastIP)) {
		broadcastIp = "localhost"; // IP
		broadcastPort = 12345; // Port
	}
	else {
		ifstream fin;
		string str;
		fin.open(pathBroadcastIP.c_str());
		for (int i = 0; i < 2; i++) {
			string str; //declare a string for storage  
			getline(fin, str); //get a line from the file, put it in the string  
			loadedBroadcastIPData.push_back(str);
		}
		fin.close();
		broadcastIp = loadedBroadcastIPData[0];
		broadcastPort = atoi(loadedBroadcastIPData[1].c_str());
	}

	// GUI
	iconPath = "gui/";
	iconSize = 50;
	iconSizeLarge = 70;
	iconSizeHuge = 100;
	iconPadding = 5;
	piscoreIcon.load(iconPath + "icon_piscore");
	editButton.load(iconPath + "black_pencil");
	resetButton.load(iconPath + "black_reset");
	playButton.load(iconPath + "black_triangleRight");
	pauseButton.load(iconPath + "black_pause");
	rewindButton.load(iconPath + "black_rewind");
	fastForwardButton.load(iconPath + "black_fastForward");
	zoomButton.load(iconPath + "black_zoom");
	shutdownButton.load(iconPath + "black_shutdown");
	homeButton.load(iconPath + "black_home");
	zeroButton.load(iconPath + "black_zero");
	plusButton.load(iconPath + "black_plus");
	minusButton.load(iconPath + "black_minus");
	upButton.load(iconPath + "black_triangleUp");
	downButton.load(iconPath + "black_triangleDown");
	leftButton.load(iconPath + "black_triangleLeft");
	cancelButton.load(iconPath + "black_cross");
	confirmButton.load(iconPath + "black_checkmark");
	launchButton.load(iconPath + "black_flash100px");
	loadButton.load(iconPath + "black_folder");;
	wizardButton.load(iconPath + "black_musicalNote");
	emptyButton.load(iconPath + "black_empty");
	checkedButton.load(iconPath + "black_checked");
	screenButton.load(iconPath + "black_monitor");
	pencilWritingButton.load(iconPath + "black_pencilWriting");
	eraserButton.load(iconPath + "black_eraser");



	dialogTimeout = 3000;
	zoomDialogp = false;
	zoomDialogSchedule = 0;
	exitDialogp = false;
	exitDialogSchedule = 0;

}

int ofApp::calculateScoreX(int frame) {
	float travel; // 0 - 1, 1 = 100% complete
	int xPos;
	if (frame == 0) {
		travel = 0;
	}
	else {
		travel = (frame / static_cast<float>(totalFrames));
	}
	xPos = round((totalPx * travel) + start);
	return xPos;
}

int ofApp::calculateScoreXFromAudio(int ms) {
	float travel;
	int xPos;
	if ((ms - audioStart) == 0) {
		travel = 0;
	}
	else {
		travel = (ms - audioStart) / (dur * 1000);
	}

	xPos = round((totalPx * travel) + start);
	return xPos;
}


//--------------------------------------------------------------
void ofApp::update() {

	if (appState == 2) {

		if (serverp) {
			if (build != "raspi") {
				ofSoundUpdate();
			}

			if (audiop) {
				if (!audio.isPlaying()) {
					playingp = false;
					audio.play();
					audio.setPaused(true);
					audio.setPosition(0.9999999);
				}
				audioMS = audio.getPositionMS();
				scoreX = calculateScoreXFromAudio(audioMS);
			}
			else {
				if (frameCounter >= totalFrames) {
					playingp = false;
				}
				if (playingp) {
					frameCounter++;
				}
				scoreX = calculateScoreX(frameCounter);
			}
			ofxOscMessage m;
			m.setAddress("/piscore/scorex");
			m.addIntArg(scoreX);
			sender.sendMessage(m, false);
		}
		else {
			while (receiver.hasWaitingMessages()) {
				// get the next message
				ofxOscMessage m;
				receiver.getNextMessage(m);

				// check for scorex message
				if (m.getAddress() == "/piscore/scorex") {
					// both the arguments are int32's
					scoreX = m.getArgAsInt32(0);
				}

			}
		}
		if (!editModep) {
			localScoreX = scoreX;
		}
	}

	elapsedTimeMS = ofGetElapsedTimeMillis();
	if (zoomDialogp && (elapsedTimeMS > zoomDialogSchedule)) {
		zoomDialogp = false;
	}
	if (exitDialogp && (elapsedTimeMS > exitDialogSchedule)) {
		exitDialogp = false;
	}

}

//--------------------------------------------------------------
void ofApp::draw() {
	if ((editModep && appState == 2) || appState == 1) ofBackground(230);
	else ofBackground(ofColor::white);

	if (appState == 0) {
		// Launch
		ofSetColor(255);
		ofFill();
		ofCircle(
			round(ofGetWidth() * 0.5),
			round(ofGetHeight() * 0.5),
			round(iconSizeHuge * 0.5));
		if (!loadedScorep || !loadedScoreConfp) ofSetColor(255, notAvailableAlpha);
		else ofSetColor(255);
		launchButton.draw(
			round((ofGetWidth() - iconSizeHuge) * 0.5),
			round((ofGetHeight() - iconSizeHuge) * 0.5),
			iconSizeHuge,
			iconSizeHuge);
		if (!loadedScorep || !loadedScoreConfp) ofSetColor(0, notAvailableAlpha);
		else ofSetColor(0);
		ofDrawBitmapString("Launch PiScore Performer",
			round((ofGetWidth() + iconSizeHuge) * 0.5) + iconPadding,
			round(ofGetHeight() * 0.5) + 4);

		// Load
		ofSetColor(255);
		ofFill();
		ofCircle(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		loadButton.draw(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */),
			iconPadding + ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
		if (loadedScorep) {
			ofSetColor(0);
			if (serverp) {
				if (build == "raspi") {
					ofDrawBitmapString("Score loaded: " + ofFilePath::getFileName(pathLoadedScore),
						iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
						iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5) + 4);
				}
				else if (audiop) {
					ofDrawBitmapString("Score loaded: " + ofFilePath::getFileName(pathLoadedScore) + ".\nAudio loaded: true",
						iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
						iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5) - 2);
				}
				else {
					ofDrawBitmapString("Score loaded: " + ofFilePath::getFileName(pathLoadedScore) + ".\nAudio loaded: false",
						iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
						iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5) - 2);
				}
			}
			else {
				ofDrawBitmapString("Score loaded: " + ofFilePath::getFileName(pathLoadedScore),
					iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
					iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5) + 4);
			}
		}
		else {
			ofSetColor(255, 0, 0);
			ofDrawBitmapString("Please load score (PNG/JPG/GIF)",
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5) + 4);
		}

		// Wizard
		ofSetColor(255);
		ofFill();
		ofCircle(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		if (!loadedScorep) ofSetColor(255, notAvailableAlpha);
		else ofSetColor(255);
		wizardButton.draw(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */),
			iconPadding + ((iconPadding + iconSize) * 1 /* vPos */),
			iconSize,
			iconSize);
		if (loadedScoreConfp) {
			ofSetColor(0);
			ofDrawBitmapString("Edit score setup",
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + round(iconSize * 0.5) + 4);
		}
		else {
			if (loadedScorep) {
				ofSetColor(255, 0, 0);
				ofDrawBitmapString(ofFilePath::getFileName(pathLoadedScoreConf) + " not found. Please run score setup",
					iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
					iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + round(iconSize * 0.5) + 4);
			}
		}

		// Launch as Server?
		ofSetColor(255);
		ofFill();
		ofCircle(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		if (serverp) {
			checkedButton.draw(
				iconPadding + ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */),
				iconSize,
				iconSize);
			ofSetColor(0);
			ofDrawBitmapString("Launching as server",
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + round(iconSize * 0.5) + 4);
		}
		else {
			emptyButton.draw(
				iconPadding + ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */),
				iconSize,
				iconSize);
			ofSetColor(0);
			ofDrawBitmapString("Launch as server",
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + round(iconSize * 0.5) + 4);
		}

		// Edit IP
		ofSetColor(255);
		ofFill();
		ofCircle(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		editButton.draw(
			iconPadding + ((iconPadding + iconSize) * 0 /* hPos */),
			iconPadding + ((iconPadding + iconSize) * 3 /* vPos */),
			iconSize,
			iconSize);
		ofSetColor(0);
		if (serverp) {
			ofDrawBitmapString("Broadcast IP:   " + broadcastIp + "\nBroadcast Port: " + to_string(broadcastPort),
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + round(iconSize * 0.5) - 2);
		}
		else {
			ofDrawBitmapString("Listening Port: " + to_string(broadcastPort),
				iconPadding + ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + round(iconSize * 0.5) + 4);
		}

		// Screen Settings
		ofSetColor(255);
		ofFill();
		ofCircle(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		screenButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
			iconPadding + ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);

		if (ipEditp) {
			ofSetColor(255, semiTransparent);
			ofRect(
				0,
				0,
				ofGetWidth(),
				ofGetHeight()
			);

			ofSetColor(255, 255, 0, semiTransparent);
			ofRect(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - iconPadding - 40,
				(iconSize + iconPadding) * 5 + iconSize,
				40
			);

			ofSetColor(0);
			if (broadcastIpStep == 0) {
				ofDrawBitmapString(
					"Enter broadcast IP:",
					((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
					ofGetHeight() - iconPadding - iconSize - iconPadding - 25);
			}
			else if (broadcastIpStep == 1) {
				if (serverp) {
					ofDrawBitmapString(
						"Enter broadcast port:",
						((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
						ofGetHeight() - iconPadding - iconSize - iconPadding - 25);
				}
				else {
					ofDrawBitmapString(
						"Enter listening port:",
						((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
						ofGetHeight() - iconPadding - iconSize - iconPadding - 25);
				}
			}

			ofSetColor(ofColor::red);
			ofDrawBitmapString(
				tempTextBuffer,
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
				ofGetHeight() - iconPadding - iconSize - iconPadding - 5);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(100, 255, 100, semiTransparent);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			checkedButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);
		}

		if (screenSettingsp) {
			ofSetColor(255, semiTransparent);
			ofRect(
				0,
				0,
				ofGetWidth(),
				ofGetHeight()
			);

			ofSetColor(255, 255, 0, semiTransparent);
			ofRect(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - iconPadding - 40,
				(iconSize + iconPadding) * 5 + iconSize,
				40
			);

			ofSetColor(0);
			if (screenSettingsStep == 0) {
				ofDrawBitmapString(
					"Enter window width:",
					((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
					ofGetHeight() - iconPadding - iconSize - iconPadding - 25);
			}
			else if (screenSettingsStep == 1) {
				ofDrawBitmapString(
					"Enter window height:",
					((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
					ofGetHeight() - iconPadding - iconSize - iconPadding - 25);
			}

			ofSetColor(ofColor::red);
			ofDrawBitmapString(
				tempTextBuffer,
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
				ofGetHeight() - iconPadding - iconSize - iconPadding - 5);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(100, 255, 100, semiTransparent);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			checkedButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);
		}

		if (viewLicensep) {
			ofSetColor(255);
			ofRect(
				0,
				0,
				ofGetWidth(),
				ofGetHeight()
			);

			ofSetColor(0);
			for (int i = 0; i < license.size(); i++) {
				ofDrawBitmapString(
					license[i],
					(iconPadding + iconSize * 2),
					(iconPadding + iconSize * 2) + ((i + 1) * 20));
			}

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(100, 255, 100, semiTransparent);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			checkedButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);
		}



	}

	if (appState == 1) {
		if (wizardStep <= 3) {
			ofSetColor(255);

			for (int i = 0; i < scoreTilesImages.size(); i++) {
				scoreTilesImages[i].draw(-round(((scoreX)*zoom) - playheadPos) + round(scoreTilesImages[0].getWidth() * i * zoom), vOffset, round(scoreTilesImages[i].getWidth() * zoom), round(scoreTilesImages[i].getHeight() * zoom));
			}

			// Draw playhead
			ofSetColor(255, 0, 0, semiTransparent);
			ofSetLineWidth(5);
			ofLine(playheadPos, 0, playheadPos, ofGetHeight());

			ofSetColor(255, 0, 0);
			ofDrawBitmapString(
				scoreX,
				iconPadding,
				ofGetHeight() - iconPadding);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			rewindButton.draw(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			leftButton.draw(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 1 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			minusButton.draw(
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			plusButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 0 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			playButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 1 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);

			ofSetColor(255);
			ofFill();
			ofCircle(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			fastForwardButton.draw(
				((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 2 /* hPos */),
				ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);
		}

		ofSetColor(255, 255, 0, semiTransparent);
		ofRect(
			((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - iconPadding - 40,
			(iconSize + iconPadding) * 5 + iconSize,
			40
		);

		ofSetColor(0);
		ofDrawBitmapString(
			wizardText[wizardStep],
			((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
			ofGetHeight() - iconPadding - iconSize - iconPadding - 25);

		if (wizardStep > 3) {
			ofSetColor(ofColor::red);
			ofDrawBitmapString(
				tempTextBuffer,
				((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + 5,
				ofGetHeight() - iconPadding - iconSize - iconPadding - 5);
		}

		ofSetColor(255);
		ofFill();
		ofCircle(
			((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(100, 255, 100, semiTransparent);
		ofFill();
		ofCircle(
			((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + round(iconSize * 0.5),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		checkedButton.draw(
			((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
	}

	if (appState == 2) {
		ofSetColor(255);
		for (int i = 0; i < scoreTilesImages.size(); i++) {
			scoreTilesImages[i].draw(-round(((localScoreX)*zoom) - playheadPos) + round(scoreTilesImages[0].getWidth() * i * zoom), vOffset, round(scoreTilesImages[i].getWidth() * zoom), round(scoreTilesImages[i].getHeight() * zoom));
		}
		for (int i = 0; i < canvasTilesImages.size(); i++) {
			canvasTilesImages[i].draw(-round(((localScoreX)*zoom) - playheadPos) + round(canvasTilesImages[0].getWidth() * i * zoom), vOffset, round(canvasTilesImages[i].getWidth() * zoom), round(canvasTilesImages[i].getHeight() * zoom));
		}

		//score.draw(-round((localScoreX*zoom) - playheadPos), vOffset, round(score.getWidth() * zoom), round(score.getHeight() * zoom));
		//canvas.draw(-round((localScoreX*zoom) - playheadPos), vOffset, round(scoreTotalWidth * zoom), round(scoreTotalHeight * zoom));
		if ((clefsEnd - clefsStart) > 0) {
			if ((round(localScoreX*zoom) - playheadPos) > clefsStart * zoom) {
				clefs.draw(0, vOffset, round(clefs.getWidth() * zoom), round(clefs.getHeight() * zoom));
			}
			if (editModep) {
				ofSetColor(230, semiTransparent);
				ofRect(
					0,
					0,
					clefs.getWidth() * zoom,
					ofGetHeight());
			}
		}

		// Draw ID markers
		ofSetColor(0);
		for (
			int i = 0, j = 0, vPos = (ofGetHeight() - iconPadding);
			i < (scoreTotalWidth - playheadPos);
			i += ((end - start) / dur * 10), j++)
		{
			if (j == 0) {
				ofDrawBitmapString(
					"ID:",
					round((i - localScoreX + start) * zoom) + playheadPos - 30,
					vPos);
			}
			ofDrawBitmapString(
				j,
				round((i - localScoreX + start) * zoom) + playheadPos,
				vPos);
		}

		if (!editModep) {
			// Draw playhead
			ofSetColor(255, 0, 0, semiTransparent);
			ofSetLineWidth(5);
			ofLine(playheadPos, 0, playheadPos, ofGetHeight());
		}


		/////////
		// GUI //
		/////////

		if (editModep) {
			ofSetColor(230, semiTransparent);
			ofRect(
				ofGetWidth() - iconSize - iconPadding * 2,
				0,
				iconSize + iconPadding * 2,
				ofGetHeight());
		}

		// Edit mode
		if (!(serverp && playingp)) {
			if (editModep) ofSetColor(255, 0, 0);
			else ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			editButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 0 /* vPos */),
				iconSize,
				iconSize);
		}

		// Reset / pencil
		if (serverp || editModep) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			if (editModep && !eraserp && !zoomDialogp && !exitDialogp) {
				ofSetColor(255, 0, 0, semiTransparent);
				ofFill();
				ofCircle(
					ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
					iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + round(iconSize * 0.5),
					round(iconSize * 0.5));
			}
		}
		ofSetColor(255);
		if (editModep) {
			pencilWritingButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 1 /* vPos */),
				iconSize,
				iconSize);
		}
		else {
			if (serverp) {
				resetButton.draw(
					ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
					iconPadding + ((iconPadding + iconSize) * 1 /* vPos */),
					iconSize,
					iconSize);
			}
		}

		// Play/Pause / Eraser
		if (serverp || editModep) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			if (editModep && eraserp && !zoomDialogp && !exitDialogp) {
				ofSetColor(255, 0, 0, semiTransparent);
				ofFill();
				ofCircle(
					ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
					iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + round(iconSize * 0.5),
					round(iconSize * 0.5));
			}
		}
		ofSetColor(255);
		if (editModep) {
			eraserButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 2 /* vPos */),
				iconSize,
				iconSize);
		}
		else {
			if (serverp) {
				if (playingp) {
					pauseButton.draw(
						ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
						iconPadding + ((iconPadding + iconSize) * 2 /* vPos */),
						iconSize,
						iconSize);
				}
				else {
					playButton.draw(
						ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
						iconPadding + ((iconPadding + iconSize) * 2 /* vPos */),
						iconSize,
						iconSize);
				}
			}
		}

		// Rewind
		if (serverp || editModep) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			rewindButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 3 /* vPos */),
				iconSize,
				iconSize);
		}

		// Fast Forward
		if (serverp || editModep) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			fastForwardButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */),
				iconSize,
				iconSize);
		}

		// Zoom
		ofSetColor(255);
		ofFill();
		ofCircle(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
			iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		zoomButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
			iconPadding + ((iconPadding + iconSize) * 5 /* vPos */),
			iconSize,
			iconSize);

		// Reset Zoom
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			zeroButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 5 /* vPos */),
				iconSize,
				iconSize);
		}

		// Plus Zoom
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			plusButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */),
				iconSize,
				iconSize);
		}

		// Minus Zoom
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 6 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			minusButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 6 /* vPos */),
				iconSize,
				iconSize);
		}

		// Reset vOffset
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			zeroButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 5 /* vPos */),
				iconSize,
				iconSize);
		}

		// Plus vOffset
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			upButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 4 /* vPos */),
				iconSize,
				iconSize);
		}

		// Minus vOffset
		if (zoomDialogp) {
			ofSetColor(255);
			ofFill();
			ofCircle(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
				iconPadding + ((iconPadding + iconSize) * 6 /* vPos */) + round(iconSize * 0.5),
				round(iconSize * 0.5));
			ofSetColor(255);
			downButton.draw(
				ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
				iconPadding + ((iconPadding + iconSize) * 6 /* vPos */),
				iconSize,
				iconSize);
		}

		if (editModep) {
			if (mouseXScreen < (ofGetWidth() - iconPadding - iconSize) && !zoomDialogp && !exitDialogp) {
				if (mousePressedp) ofSetColor(255, 0, 0, 200);
				else ofSetColor(255, 0, 0, 78);
				ofRect(
					mouseXScreen - (penSize * zoom * 0.5),
					mouseYScreen - (penSize * zoom * 0.5),
					penSize * zoom,
					penSize * zoom);
			}
		}
	}

	// Exit
	ofSetColor(255);
	ofFill();
	ofCircle(
		ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + round(iconSize * 0.5),
		ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
		round(iconSize * 0.5));
	ofSetColor(255);
	if (appState == 0) {
		shutdownButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
	}
	else {
		homeButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
	}

	// Exit Cancel
	if (exitDialogp) {
		ofSetColor(255);
		ofFill();
		ofCircle(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + round(iconSize * 0.5),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		cancelButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
	}

	// Exit Confirm
	if (exitDialogp) {
		ofSetColor(255);
		ofFill();
		ofCircle(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + round(iconSize * 0.5),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + round(iconSize * 0.5),
			round(iconSize * 0.5));
		ofSetColor(255);
		confirmButton.draw(
			ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */),
			ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */),
			iconSize,
			iconSize);
	}

	// Draw PiScore icon
	if (appState == 0) {
		ofSetColor(0);
		ofDrawBitmapString(
			"PiScore v" + appVersionID,
			iconSizeLarge,
			ofGetHeight() - iconPadding);
		ofSetColor(255);
		piscoreIcon.draw(
			iconPadding,
			ofGetHeight() - iconPadding - iconSizeLarge,
			iconSizeLarge,
			iconSizeLarge);
	}

	// Draw cursor
		ofSetColor(ofColor::black);
		ofSetLineWidth(2);
		ofDrawLine(mouseXScreen - 2, mouseYScreen, mouseXScreen - 10, mouseYScreen);
		ofDrawLine(mouseXScreen + 2, mouseYScreen, mouseXScreen + 10, mouseYScreen);
		ofDrawLine(mouseXScreen, mouseYScreen - 2, mouseXScreen, mouseYScreen - 10);
		ofDrawLine(mouseXScreen, mouseYScreen + 2, mouseXScreen, mouseYScreen + 10);

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	if (
		(appState == 1 && wizardStep > 3) ||
		(appState == 0 && ipEditp) ||
		(appState == 0 && screenSettingsp)
		) {
		if (
			key == '0' ||
			key == '1' ||
			key == '2' ||
			key == '3' ||
			key == '4' ||
			key == '5' ||
			key == '6' ||
			key == '7' ||
			key == '8' ||
			key == '9' ||
			key == '.')
		{
			tempTextBuffer += key;
		}

		if (key == OF_KEY_BACKSPACE) {
			if (tempTextBuffer.size() > 0) {
				tempTextBuffer.pop_back();
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
	mouseXScreen = x;
	mouseYScreen = y;
	if (appState == 2) {
		pMouseX = mouseX; // Get the previous mouseX value
		pMouseY = mouseY; // Get the previous mouseY value
		mouseX = localScoreX + ((x - playheadPos) / zoom);
		mouseX = std::min(mouseX, scoreTotalWidth); // Limit max value
		mouseY = (y / zoom) - (vOffset / zoom); // Update mouseY
	}
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
	mouseXScreen = x;
	mouseYScreen = y;
	if (appState == 2 && editModep) {
		pMouseX = mouseX; // Get the previous mouseX value
		pMouseY = mouseY; // Get the previous mouseY value
		mouseX = localScoreX + ((x - playheadPos) / zoom);
		mouseX = std::min(mouseX, scoreTotalWidth); // Limit max value
		mouseY = (y / zoom) - (vOffset / zoom); // Update mouseY

												// Bresenham's line algorithm
												/*void line(int x0, int y0, int x1, int y1) {

												int dx = abs(x1 - x0), sx = x0<x1 ? 1 : -1;
												int dy = abs(y1 - y0), sy = y0<y1 ? 1 : -1;
												int err = (dx>dy ? dx : -dy) / 2, e2;

												for (;;) {
												setPixel(x0, y0);
												if (x0 == x1 && y0 == y1) break;
												e2 = err;
												if (e2 >-dx) { err -= dy; x0 += sx; }
												if (e2 < dy) { err += dx; y0 += sy; }
												}
												}*/
		
		int tilesWidth = (canvasTilesImages[0].getWidth());
		int currentTile = mouseX / tilesWidth;

		int x1 = mouseX, y1 = mouseY, x0 = pMouseX, y0 = pMouseY;

		int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
		int dy = abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
		int err = (dx > dy ? dx : -dy) / 2, e2;

		for (;;) {
			for (int i = x0; i < x0 + penSize; i++) {
				for (int j = y0; j < y0 + penSize; j++) {
					if (mouseXScreen > clefs.getWidth() * zoom &&
						mouseXScreen < (ofGetWidth() - iconPadding * 2 - iconSize) &&
						!zoomDialogp &&
						!exitDialogp
						) { // Don't draw under icon panel
						if (// Protect against out-of-bounds drawing
							i - (penSize * 0.5) >= tilesWidth * currentTile &&
							i - (penSize * 0.5) < tilesWidth * currentTile + canvasTilesImages[currentTile].getWidth() &&
							j - (penSize * 0.5) >= 0 &&
							j - (penSize * 0.5) < canvasTilesImages[currentTile].getHeight())
						{
							canvasTilesImages[currentTile].setColor(
								i - (penSize * 0.5),
								j - (penSize * 0.5) - currentTile,
								penColor);
						}
					}
				}
			}
			if (x0 == x1 && y0 == y1) break;
			e2 = err;
			if (e2 > -dx) { err -= dy; x0 += sx; }
			if (e2 < dy) { err += dx; y0 += sy; }
		}
		
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	mousePressedp = true;

	if (appState == 2 && editModep) {
		mouseXScreen = x;
		mouseYScreen = y;
		pMouseX = mouseX; // Get the previous mouseX value
		pMouseY = mouseY; // Get the previous mouseY value
		mouseX = localScoreX + ((x - playheadPos) / zoom);
		mouseX = std::min(mouseX, scoreTotalWidth); // Limit max value
		mouseY = (y / zoom) - (vOffset / zoom); // Update mouseY

		int tilesWidth = canvasTilesImages[0].getWidth();
		int currentTile = mouseX / tilesWidth;

		for (int i = mouseX; i < mouseX + penSize; i++) {
			for (int j = mouseY; j < mouseY + penSize; j++) {
				if (mouseXScreen > clefs.getWidth() * zoom &&
					mouseXScreen < (ofGetWidth() - iconPadding * 2 - iconSize) &&
					!zoomDialogp &&
					!exitDialogp
					) { // Don't draw under icon panel
					if (// Protect against out-of-bounds drawing
						i - (penSize * 0.5) >= tilesWidth * currentTile &&
						i - (penSize * 0.5) < tilesWidth * currentTile + canvasTilesImages[currentTile].getWidth() &&
						j - (penSize * 0.5) >= 0 &&
						j - (penSize * 0.5) < canvasTilesImages[0].getHeight())
					{
						canvasTilesImages[currentTile].setColor(
							i - (penSize * 0.5),
							j - (penSize * 0.5) - currentTile,
							penColor);
					}
				}
			}
		}
		for (int i = 0; i < canvasTilesImages.size(); i++) {
			canvasTilesImages[i].update();
		}
	}

	if (
		(appState == 0) &&
		(!ipEditp) &&
		(!screenSettingsp) &&
		(!viewLicensep)
		) {

		// Launch
		if (loadedScorep && loadedScoreConfp &&
			(x > round((ofGetWidth() - iconSizeHuge) * 0.5)) &&
			(x < round((ofGetWidth() - iconSizeHuge) * 0.5) + iconSizeHuge) &&
			(y > round((ofGetHeight() - iconSizeHuge) * 0.5)) &&
			(y < round((ofGetHeight() - iconSizeHuge) * 0.5) + iconSizeHuge)
			)
		{
			appState = 2;

			if (serverp) sender.setup(broadcastIp, broadcastPort);
			else receiver.setup(broadcastPort);

			exitDialogp = false;
			exitDialogSchedule = elapsedTimeMS;
			zoomDialogp = false;
			zoomDialogSchedule = elapsedTimeMS;
			editModep = false;
			eraserp = false;

			totalPx = end - start;
			totalFrames = ceil(dur * fps);
			frameCounter = round(-(preroll*fps));

			scoreTotalWidth = 0; // Reset totalwidth for new calculation
			scoreTotalHeight = 0; // Reset totalheight for new calculation
			for (int i = 0; i < scoreTilesPaths.size(); i++) {
				ofImage tmpImg;
				tmpImg.load(scoreTilesPaths[i]);
				scoreTilesImages.push_back(tmpImg);
				scoreTotalWidth += tmpImg.getWidth();
				if (i == 0) scoreTotalHeight = tmpImg.getHeight(); // Calculate height from first tile only
			}
			if ((clefsEnd - clefsStart) > 0) {
				clefs.cropFrom(scoreTilesImages[0], clefsStart, 0, (clefsEnd - clefsStart), scoreTotalHeight);
			}
			for (int i = 0; i < canvasTilesPaths.size(); i++) {
				ofImage tmpImg;
				if (ofFile::doesFileExist(canvasTilesPaths[i])) {
					tmpImg.load(canvasTilesPaths[i]);
					//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
				else {
					tmpImg.allocate(scoreTilesImages[0].getWidth(), scoreTilesImages[i].getHeight(), OF_IMAGE_COLOR_ALPHA);
					//std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
				canvasTilesImages.push_back(tmpImg);
			}

			scoreFitToHeight = ofGetHeight() / static_cast<float>(scoreTotalHeight);

			if (serverp && audiop && build != "raspi") {
				audio.load(pathAudio);
				audio.setVolume(1.0);
				audio.setLoop(false);
				audio.setMultiPlay(false);
				audio.play();
				audio.setPaused(true);
				audio.setPositionMS(audioStart - (preroll * 1000));
			}

			playingp = false;
		}

		// Load
		if (
			(x > iconPadding + ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			ofFileDialogResult loadscore = ofSystemLoadDialog("Load first score tile (PNG/JPG/GIF)");
			if (loadscore.bSuccess) {
				scoreTilesPaths.clear();
				canvasTilesPaths.clear();
				string str = loadscore.getPath();
				string ext = ofFilePath::getFileExt(str);
				ofDirectory dir(ofFilePath::getEnclosingDirectory(str));
				//load show files with same extension
				dir.allowExt(ext);
				//populate the directory object
				dir.listDir();
				for (int i = 0; i < dir.size(); i++) {
					string tmpPath = dir.getPath(i);
					string tmpCanvasPath = ofFilePath::getEnclosingDirectory(tmpPath) + "annotations/" + ofFilePath::getBaseName(tmpPath) + "-annotations.png";
					if (tmpPath.find("-annotations.png") == std::string::npos) { // Only load if -annotations.png is not part of the filename
						scoreTilesPaths.push_back(tmpPath);
						canvasTilesPaths.push_back(tmpCanvasPath);
					}
				}

				loadedScorep = true;
				filePrevScore.open(pathPrevScore, ofFile::WriteOnly);
				filePrevScore << str << endl;
				filePrevScore.close();
				pathLoadedScore = str;
				if (build != "raspi") {
					pathAudio = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + ".wav";
					//pathCanvas = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + "-canvas.png";
					if (ofFile::doesFileExist(pathAudio)) {
						audiop = true;
					}
					else audiop = false;
				}
				pathLoadedScoreConf = ofFilePath::getEnclosingDirectory(str) + ofFilePath::getBaseName(str) + ".piscore";
				if (ofFile::doesFileExist(pathLoadedScoreConf)) {
					loadedScoreConfp = true;
					ifstream fin; //declare a file stream  
					fin.open(pathLoadedScoreConf.c_str()); //open your text file
					for (int i = 0; i < 9; i++) {
						string str; //declare a string for storage  
						getline(fin, str); //get a line from the file, put it in the string  
						loadedScoreConfData.push_back(str);
					}
					fin.close();
					start = atoi(loadedScoreConfData[0].c_str());
					end = atoi(loadedScoreConfData[1].c_str());
					clefsStart = atoi(loadedScoreConfData[2].c_str());
					clefsEnd = atoi(loadedScoreConfData[3].c_str());
					dur = atof(loadedScoreConfData[4].c_str());
					preroll = atof(loadedScoreConfData[5].c_str());
					zoom = atof(loadedScoreConfData[6].c_str());
					vOffset = atoi(loadedScoreConfData[7].c_str());
					audioStart = atoi(loadedScoreConfData[8].c_str());
				}
				else {
					loadedScoreConfp = false;
				}
			}
		}

		// Wizard
		if (loadedScorep &&
			(x > iconPadding + ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 1 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + iconSize)
			)
		{
			appState = 1;


			exitDialogp = false;
			exitDialogSchedule = elapsedTimeMS;
			zoomDialogp = false;
			zoomDialogSchedule = elapsedTimeMS;

			scoreTotalWidth = 0; // Reset totalwidth for new calculation
			scoreTotalHeight = 0; // Reset totalheight for new calculation
			for (int i = 0; i < scoreTilesPaths.size(); i++) {
				ofImage tmpImg;
				tmpImg.load(scoreTilesPaths[i]);
				scoreTilesImages.push_back(tmpImg);
				scoreTotalWidth += tmpImg.getWidth();
				if (i == 0) scoreTotalHeight = tmpImg.getHeight(); // Calculate height from first tile only
			}

			scoreFitToHeight = ofGetHeight() / static_cast<float>(scoreTotalHeight);

			if (!loadedScoreConfp) {
				start = 0;
				end = scoreTotalWidth;
				clefsStart = 0;
				clefsEnd = 0;
				dur = 30.0;
				preroll = 4.0;
				zoom = scoreFitToHeight;
				vOffset = 0;
				audioStart = 0;
			}
			wizardStep = 0;
			scoreX = start;
		}

		// Serverp
		if (
			(x > iconPadding + ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 2 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + iconSize)
			)
		{
			if (!serverp) {
				serverp = true;
				fileServerp.open(pathServerp, ofFile::WriteOnly);
				fileServerp << "1" << endl;
				fileServerp.close();
			}
			else {
				serverp = false;
				fileServerp.open(pathServerp, ofFile::WriteOnly);
				fileServerp << "0" << endl;
				fileServerp.close();
			}
		}

		// Edit IP
		if (
			(x > iconPadding + ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < iconPadding + ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 3 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + iconSize)
			)
		{
			if (serverp) {
				broadcastIpStep = 0;
				tempTextBuffer = broadcastIp;
				ipEditp = true;
			}
			else {
				broadcastIpStep = 1;
				tempBroadcastIp = broadcastIp;
				tempTextBuffer = to_string(broadcastPort);
				ipEditp = true;
			}
		}

		// Edit Screen Settings
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			screenSettingsStep = 0;
			tempTextBuffer = to_string(width);
			screenSettingsp = true;
		}

		// View License
		if (
			(x > iconPadding) &&
			(x < iconPadding + iconSizeLarge) &&
			(y > ofGetHeight() - iconPadding - iconSizeLarge) &&
			(y < (ofGetHeight() - iconPadding - iconSizeLarge) + iconSize)
			)
		{
			viewLicensep = true;
		}
	}

	if (ipEditp) {
		if (
			(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */)) &&
			(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + iconSize) &&
			(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			if (broadcastIpStep == 0) {
				tempBroadcastIp = tempTextBuffer;
				tempTextBuffer = to_string(broadcastPort);
				broadcastIpStep += 1;
			}
			else {
				tempBroadcastPort = atoi(tempTextBuffer.c_str());

				broadcastIp = tempBroadcastIp;
				broadcastPort = tempBroadcastPort;
				fileBroadcastIP.open(pathBroadcastIP, ofFile::WriteOnly);
				fileBroadcastIP << broadcastIp << endl;
				fileBroadcastIP << broadcastPort << endl;
				fileBroadcastIP.close();
				ipEditp = false;
			}
		}
	}

	if (screenSettingsp) {
		if (
			(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */)) &&
			(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + iconSize) &&
			(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			if (screenSettingsStep == 0) {
				tempWidth = atoi(tempTextBuffer.c_str());
				tempTextBuffer = to_string(height);
				screenSettingsStep += 1;
			}
			else {
				tempHeight = atoi(tempTextBuffer.c_str());

				width = tempWidth;
				height = tempHeight;
				fileScreenSettings.open(pathScreenSettings, ofFile::WriteOnly);
				fileScreenSettings << width << endl;
				fileScreenSettings << height << endl;
				fileScreenSettings.close();
				ofSetWindowShape(width, height);
				screenSettingsp = false;
			}
		}

	}

	if (viewLicensep) {
		if (
			(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */)) &&
			(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + iconSize) &&
			(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			viewLicensep = false;
		}

	}

	if (appState == 1) {
		if (wizardStep <= 3) {
			if (
				(x > ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */)) &&
				(x < ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start -= 100;
					scoreX -= 100;
				}
				else if (wizardStep == 1) {
					end -= 100;
					scoreX -= 100;
				}
				else if (wizardStep == 2) {
					clefsStart -= 100;
					scoreX -= 100;
				}
				else if (wizardStep == 3) {
					clefsEnd -= 100;
					scoreX -= 100;
				}
			}
			if (
				(x > ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 1 /* hPos */)) &&
				(x < ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start -= 20;
					scoreX -= 20;
				}
				else if (wizardStep == 1) {
					end -= 20;
					scoreX -= 20;
				}
				else if (wizardStep == 2) {
					clefsStart -= 20;
					scoreX -= 20;
				}
				else if (wizardStep == 3) {
					clefsEnd -= 20;
					scoreX -= 20;
				}
			}
			if (
				(x > ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
				(x < ((ofGetWidth() - iconPadding) / 2) - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start -= 1;
					scoreX -= 1;
				}
				else if (wizardStep == 1) {
					end -= 1;
					scoreX -= 1;
				}
				else if (wizardStep == 2) {
					clefsStart -= 1;
					scoreX -= 1;
				}
				else if (wizardStep == 3) {
					clefsEnd -= 1;
					scoreX -= 1;
				}
			}
			if (
				(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 0 /* hPos */)) &&
				(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start += 1;
					scoreX += 1;
				}
				else if (wizardStep == 1) {
					end += 1;
					scoreX += 1;
				}
				else if (wizardStep == 2) {
					clefsStart += 1;
					scoreX += 1;
				}
				else if (wizardStep == 3) {
					clefsEnd += 1;
					scoreX += 1;
				}
			}
			if (
				(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 1 /* hPos */)) &&
				(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start += 20;
					scoreX += 20;
				}
				else if (wizardStep == 1) {
					end += 20;
					scoreX += 20;
				}
				else if (wizardStep == 2) {
					clefsStart += 20;
					scoreX += 20;
				}
				else if (wizardStep == 3) {
					clefsEnd += 20;
					scoreX += 20;
				}
			}
			if (
				(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 2 /* hPos */)) &&
				(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
				(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
				(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
				)
			{
				if (wizardStep == 0) {
					start += 100;
					scoreX += 100;
				}
				else if (wizardStep == 1) {
					end += 100;
					scoreX += 100;
				}
				else if (wizardStep == 2) {
					clefsStart += 100;
					scoreX += 100;
				}
				else if (wizardStep == 3) {
					clefsEnd += 100;
					scoreX += 100;
				}
			}
		}

		// Next Wizard Step
		if (
			(x > ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */)) &&
			(x < ((ofGetWidth() + iconPadding) / 2) + ((iconPadding + iconSize) * 3 /* hPos */) + iconSize) &&
			(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			wizardStep += 1;
			if (wizardStep == 1) {
				scoreX = end;
			}
			else if (wizardStep == 2) {
				scoreX = clefsStart;
			}
			else if (wizardStep == 3) {
				if (clefsEnd < clefsStart) clefsEnd = clefsStart;
				scoreX = clefsEnd;
			}
			else if (wizardStep == 4) {
				tempTextBuffer = to_string(dur);
			}
			else if (wizardStep == 5) {
				dur = atof(tempTextBuffer.c_str());
				tempTextBuffer = to_string(preroll);
			}
			else if (wizardStep == 6) {
				preroll = atof(tempTextBuffer.c_str());
				tempTextBuffer = to_string(audioStart);
			}
			else {
				audioStart = atoi(tempTextBuffer.c_str());
				fileLoadedScoreConf.open(pathLoadedScoreConf, ofFile::WriteOnly);
				fileLoadedScoreConf << start << endl;
				fileLoadedScoreConf << end << endl;
				fileLoadedScoreConf << clefsStart << endl;
				fileLoadedScoreConf << clefsEnd << endl;
				fileLoadedScoreConf << dur << endl;
				fileLoadedScoreConf << preroll << endl;
				fileLoadedScoreConf << zoom << endl;
				fileLoadedScoreConf << vOffset << endl;
				fileLoadedScoreConf << audioStart << endl;
				fileLoadedScoreConf.close();

				loadedScoreConfp = true;
				exitDialogp = false;
				exitDialogSchedule = elapsedTimeMS;

				appState = 0;
				for (int i = 0; i < scoreTilesImages.size(); i++) {
					if (scoreTilesImages[i].isAllocated()) scoreTilesImages[i].clear();
				}
				scoreTilesImages.clear();
				canvasTilesImages.clear();
			}
		}
	}

	if (appState == 2) {
		// Editmode
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 0 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
			)
		{
			if (!(serverp && playingp)) {
				if (editModep) {
					editModep = false;
					eraserp = false;
					zoomDialogp = false;
					exitDialogp = false;
					penSize = 2;
					penColor = ofColor(0, 0);

					// Save canvas and settings (zoom and vOffset) on exit editmode
					//canvas.save(pathCanvas);

					fileLoadedScoreConf.open(pathLoadedScoreConf, ofFile::WriteOnly);
					fileLoadedScoreConf << start << endl;
					fileLoadedScoreConf << end << endl;
					fileLoadedScoreConf << clefsStart << endl;
					fileLoadedScoreConf << clefsEnd << endl;
					fileLoadedScoreConf << dur << endl;
					fileLoadedScoreConf << preroll << endl;
					fileLoadedScoreConf << zoom << endl;
					fileLoadedScoreConf << vOffset << endl;
					fileLoadedScoreConf << audioStart << endl;
					fileLoadedScoreConf.close();
				}
				else {
					editModep = true;
					eraserp = false;
					zoomDialogp = false;
					exitDialogp = false;
					penSize = 2;
					penColor = ofColor(0, 255);

					/*for (int i = 0; i < canvasTilesPaths[i].size(); i++) {
						ofImage tmpImg;
						if (ofFile::doesFileExist(canvasTilesPaths[i])) {
							tmpImg.load(canvasTilesPaths[i]);
						}
						else {
							tmpImg.allocate((scoreTilesImages)[i].getWidth(), (scoreTilesImages)[i].getHeight(), OF_IMAGE_COLOR_ALPHA);
						}
						canvasTilesImages.push_back(tmpImg);
					}*/

				}
			}
		}

		// Reset / Pencil
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 1 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 1 /* vPos */) + iconSize)
			)
		{
			if (editModep) {
				zoomDialogp = false;
				exitDialogp = false;
				eraserp = false;
				penSize = 2;
				penColor = ofColor(0, 255);
			}
			else {
				if (serverp) {
					playingp = false;
					if (audiop && build != "raspi") {
						audio.setPaused(true);
						audio.setPositionMS(audioStart - (preroll * 1000));
					}
					else frameCounter = round(-(preroll*fps));
				}
			}
		}

		// Play / Pause / Erase
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 2 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 2 /* vPos */) + iconSize)
			)
		{
			if (editModep) {
				zoomDialogp = false;
				exitDialogp = false;
				eraserp = true;
				penSize = 20;
				penColor = ofColor(0, 0);
			}
			else {
				if (serverp) {
					if (!playingp) {
						playingp = true;
						if (audiop && build != "raspi") audio.setPaused(false);
					}
					else {
						playingp = false;
						if (audiop && build != "raspi") audio.setPaused(true);
					}
				}
			}
		}

		// Rewind
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 3 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 3 /* vPos */) + iconSize)
			)
		{
			if (serverp) {
				if (!editModep) {
					if (audiop && build != "raspi") {
						audio.setPositionMS(audio.getPositionMS() - 10000);
					}
					else {
						frameCounter -= round(fps * 10);
					}
				}
				else {
					localScoreX -= round((ofGetWidth() - playheadPos) * 0.5 / zoom);
				}
			}
			else { // !serverp
				if (editModep) {
					localScoreX -= round((ofGetWidth() - playheadPos) * 0.5 / zoom);
				}
			}
		}

		// Fast Forward
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 4 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + iconSize)
			)
		{
			if (serverp) {
				if (!editModep) {
					if (audiop && build != "raspi") {
						audio.setPositionMS(audio.getPositionMS() + 10000);
					}
					else {
						frameCounter += round(fps * 10);
					}
				}
				else {
					localScoreX += round((ofGetWidth() - playheadPos) * 0.5 / zoom);
				}
			}
			else { // !serverp
				if (editModep) {
					localScoreX += round((ofGetWidth() - playheadPos) * 0.5 / zoom);
				}
			}
		}

		// Zoom
		if (
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 5 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + iconSize)
			)
		{
			zoomDialogp = !zoomDialogp;
			if (!zoomDialogp) {
				zoomDialogSchedule = elapsedTimeMS;
			}
			else {
				zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			}
		}

		// Zoom Reset
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 5 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			zoom = scoreFitToHeight;
			vOffset = 0;
		}

		// Zoom Plus
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 4 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			zoom *= 1.25;
		}

		// Zoom Minus
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 6 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 6 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			zoom *= 0.75;
		}

		// vOffset Reset
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 5 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 5 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			vOffset = 0;
		}

		// vOffset Plus
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 4 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 4 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			vOffset += 50;
		}

		// vOffset Minus
		if (zoomDialogp &&
			(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */)) &&
			(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
			(y > iconPadding + ((iconPadding + iconSize) * 6 /* vPos */)) &&
			(y < iconPadding + ((iconPadding + iconSize) * 6 /* vPos */) + iconSize)
			)
		{
			zoomDialogSchedule = elapsedTimeMS + dialogTimeout;
			vOffset -= 50;
		}
	}

	// Exit
	if (
		(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */)) &&
		(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* hPos */) + iconSize) &&
		(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
		(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
		)
	{
		exitDialogp = !exitDialogp;
		if (!exitDialogp) {
			exitDialogSchedule = elapsedTimeMS;
		}
		else {
			exitDialogSchedule = elapsedTimeMS + dialogTimeout;
		}
	}

	// Exit Cancel
	if (exitDialogp &&
		(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */)) &&
		(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 1 /* hPos */) + iconSize) &&
		(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
		(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
		)
	{
		exitDialogp = false;
		exitDialogSchedule = elapsedTimeMS;
	}

	// Exit Confirm
	if (exitDialogp &&
		(x > ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */)) &&
		(x < ofGetWidth() - iconPadding - iconSize - ((iconPadding + iconSize) * 2 /* hPos */) + iconSize) &&
		(y > ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */)) &&
		(y < ofGetHeight() - iconPadding - iconSize - ((iconPadding + iconSize) * 0 /* vPos */) + iconSize)
		)
	{
		if (appState == 0) ofExit();
		else {
			exitDialogp = false;
			exitDialogSchedule = elapsedTimeMS;

			if (appState == 2) { // Save canvas and settings (zoom and vOffset) on exit to launcher
				for (int i = 0; i < canvasTilesImages.size(); i++) {
					canvasTilesImages[i].save(canvasTilesPaths[i]);
				}

				fileLoadedScoreConf.open(pathLoadedScoreConf, ofFile::WriteOnly);
				fileLoadedScoreConf << start << endl;
				fileLoadedScoreConf << end << endl;
				fileLoadedScoreConf << clefsStart << endl;
				fileLoadedScoreConf << clefsEnd << endl;
				fileLoadedScoreConf << dur << endl;
				fileLoadedScoreConf << preroll << endl;
				fileLoadedScoreConf << zoom << endl;
				fileLoadedScoreConf << vOffset << endl;
				fileLoadedScoreConf << audioStart << endl;
				fileLoadedScoreConf.close();
			}

			appState = 0;

			if (audio.isLoaded()) audio.unload();
			for (int i = 0; i < scoreTilesImages.size(); i++) {
				if (scoreTilesImages[i].isAllocated()) scoreTilesImages[i].clear();
			}
			for (int i = 0; i < canvasTilesImages.size(); i++) {
				if (canvasTilesImages[i].isAllocated()) canvasTilesImages[i].clear();
			}
			if (clefs.isAllocated()) clefs.clear();
			scoreTilesImages.clear();
			canvasTilesImages.clear();
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
	mousePressedp = false;

	if (appState == 2) {
		for (int i = 0; i < canvasTilesImages.size(); i++) {
			canvasTilesImages[i].update();
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	ofSetWindowShape(width, height);
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
