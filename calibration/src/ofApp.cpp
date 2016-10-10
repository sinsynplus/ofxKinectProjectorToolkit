#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	testing = false;
	
	bUseRegistration = true;
	bUseMirroring = false;
	
	nearClip = 500;
	farClip = 6000;
	
	w = 640;
	h = 480;
	
	chessboardSize = 300;
	chessboardX = 5;
	chessboardY = 4;
	
#ifdef USE_ASTRA
    astra.setup();
    
    astra.enableRegistration(bUseRegistration);
    astra.enableMirroring(bUseMirroring);
    
    astra.setDepthClipping(nearClip, farClip);
    
    astra.initVideoGrabber();
    
    astra.initDepthStream();
    astra.initPointStream();
    //astra.initHandStream();
    
    w = astra.getDepthImage().getWidth();
    h = astra.getDepthImage().getHeight();
#else
    kinect.setRegistration(bUseRegistration);
    kinect.init();
    kinect.open();
    kinect.setDepthClipping(nearClip, farClip);
    
    if (kinect.isConnected()) {
        w = kinect.width;
        h = kinect.height;
    }
#endif
	
	//rgbImage = new ofxCvColorImage();
	rgbImage.allocate(w, h);
	fboChessboard.allocate(PROJECTOR_RESOLUTION_X, PROJECTOR_RESOLUTION_Y, GL_RGBA);
}

//--------------------------------------------------------------
void ofApp::drawChessboard(int x, int y, int chessboardSize) {
    float w = chessboardSize / chessboardX;
    float h = chessboardSize / chessboardY;
    currentProjectorPoints.clear();
    fboChessboard.begin();
    ofClear(255, 0);
    ofSetColor(0);
    ofTranslate(x, y);
    for (int j=0; j<chessboardY; j++) {
        for (int i=0; i<chessboardX; i++) {
            int x0 = ofMap(i, 0, chessboardX, 0, chessboardSize);
            int y0 = ofMap(j, 0, chessboardY, 0, chessboardSize);
            if (j>0 && i>0) {
                currentProjectorPoints.push_back(ofVec2f(
                    ofMap(x+x0, 0, fboChessboard.getWidth(), 0, 1),
                    ofMap(y+y0, 0, fboChessboard.getHeight(), 0, 1)
                ));
            }
            if ((i+j)%2==0) ofRect(x0, y0, w, h);
        }
    }
    ofSetColor(255);
    fboChessboard.end();
}

//--------------------------------------------------------------
void ofApp::drawTestingPoint(ofVec2f projectedPoint) {
    float ptSize = ofMap(sin(ofGetFrameNum()*0.1), -1, 1, 3, 40);
    fboChessboard.begin();
    ofBackground(255);
    ofSetColor(0, 255, 0);
    ofCircle(
             ofMap(projectedPoint.x, 0, 1, 0, fboChessboard.getWidth()),
             ofMap(projectedPoint.y, 0, 1, 0, fboChessboard.getHeight()),
             ptSize);
    ofSetColor(255);
    fboChessboard.end();
}

//--------------------------------------------------------------
void ofApp::addPointPair() {
    int nDepthPoints = 0;
    for (int i=0; i<cvPoints.size(); i++) {
#ifdef USE_ASTRA
        ofVec3f worldPoint = astra.getWorldCoordinateAt(cvPoints[i].x, cvPoints[i].y);
#else
        ofVec3f worldPoint = kinect.getWorldCoordinateAt(cvPoints[i].x, cvPoints[i].y);
#endif
        if (worldPoint.z > 0)   nDepthPoints++;
    }
    if (nDepthPoints == (chessboardX-1)*(chessboardY-1)) {
        for (int i=0; i<cvPoints.size(); i++) {
#ifdef USE_ASTRA
            ofVec3f worldPoint = astra.getWorldCoordinateAt(cvPoints[i].x, cvPoints[i].y);
#else
            ofVec3f worldPoint = kinect.getWorldCoordinateAt(cvPoints[i].x, cvPoints[i].y);
#endif
            pairsKinect.push_back(worldPoint);
            pairsProjector.push_back(currentProjectorPoints[i]);
        }
        resultMessage = "Added " + ofToString((chessboardX-1)*(chessboardY-1)) + " points pairs.";
        resultMessageColor = ofColor(0, 255, 0);
    } else {
        resultMessage = "Points not added because not all chessboard\npoints' depth known. Try re-positionining.";
        resultMessageColor = ofColor(255, 0, 0);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
#ifdef USE_ASTRA
    astra.update();
    if (astra.isFrameNew())
#else
    kinect.update();
    if (kinect.isFrameNew())
#endif
    {
#ifdef USE_ASTRA
        rgbImage.setFromPixels(astra.getColorImage().getPixels().getData(), w, h);
#else
        rgbImage.setFromPixels(kinect.getPixels().getData(), w, h);
#endif
        if (testing) {
            ofVec2f t = ofVec2f(min(kinect.getWidth()-1,testPoint.x), min(kinect.getHeight()-1,testPoint.y));
#ifdef USE_ASTRA
            ofVec3f worldPoint = astra.getWorldCoordinateAt(t.x, t.y);
#else
            ofVec3f worldPoint = kinect.getWorldCoordinateAt(t.x, t.y);
#endif
            ofVec2f projectedPoint = kpt.getProjectedPoint(worldPoint);
            drawTestingPoint(projectedPoint);
        }
        else {
            drawChessboard(ofGetMouseX(), ofGetMouseY(), chessboardSize);
            cvRgbImage = ofxCv::toCv(rgbImage.getPixels());
            cv::Size patternSize = cv::Size(chessboardX-1, chessboardY-1);
            int chessFlags = cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FAST_CHECK;
            bool foundChessboard = findChessboardCorners(cvRgbImage, patternSize, cvPoints, chessFlags);
            if(foundChessboard) {
                cv::Mat gray;
                cvtColor(cvRgbImage, gray, CV_RGB2GRAY);
                cornerSubPix(gray, cvPoints, cv::Size(11, 11), cv::Size(-1, -1),
                             cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
                drawChessboardCorners(cvRgbImage, patternSize, cv::Mat(cvPoints), foundChessboard);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    rgbImage.draw(0, 0);
#ifdef USE_ASTRA
    astra.drawDepth(10, 490, 320, 240);
#else
    kinect.drawDepth(10, 490, 320, 240);
#endif
    
    ofSetColor(0);
    if (testing) {
        ofDrawBitmapString("Click on the image to test a point in the RGB image.", 340, 510);
        ofDrawBitmapString("The projector should place a green dot on the corresponding point.", 340, 530);
        ofDrawBitmapString("Press the 's' key to save the calibration.", 340, 550);
        if (saved) {
            ofDrawBitmapString("Calibration saved.", 340, 590);
        }
        ofSetColor(255, 0, 0);
        float ptSize = ofMap(cos(ofGetFrameNum()*0.1), -1, 1, 3, 40);
        ofCircle(testPoint.x, testPoint.y, ptSize);
    } else {
        ofDrawBitmapString("Position the chessboard using the mouse.", 340, 510);
        ofDrawBitmapString("Adjust the size of the chessboard using the 'q' and 'w' keys.", 340, 530);
        ofDrawBitmapString("Press the spacebar to save a set of point pairs.", 340, 550);
        ofDrawBitmapString("Press the 'c' key to calibrate.", 340, 570);
        ofSetColor(resultMessageColor);
        ofDrawBitmapString(resultMessage, 340, 610);
        ofSetColor(0);
        ofDrawBitmapString(ofToString(pairsKinect.size())+" point pairs collected.", 340, 630);
    }
    ofSetColor(255);
}

void ofApp::drawSecondWindow(ofEventArgs &args){
    ofSetColor(ofColor::white);
    fboChessboard.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key==' '){
        addPointPair();
    } else if (key=='q') {
        chessboardSize -= 20;
    } else if (key=='w') {
        chessboardSize += 20;
    } else if (key=='c') {
        kpt.calibrate(pairsKinect, pairsProjector);
        testing = true;
    } else if (key=='s') {
        kpt.saveCalibration("calibration.xml");
        saved = true;
    } else if (key=='l') {
        kpt.loadCalibration("calibration.xml");
        testing = true;
    }
#ifdef USE_ASTRA
    else if (key == 'm') {
        bUseMirroring ^= 1;
        astra.enableMirroring(bUseMirroring);
    }
#endif
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    if (testing) {
        testPoint.set(min(x, kinect.width-1), min(y, kinect.height-1));
    }
}
