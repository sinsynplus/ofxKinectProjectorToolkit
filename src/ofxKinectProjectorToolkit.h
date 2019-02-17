#pragma once

#include "ofMain.h"
#include "matrix.h"
#include "matrix_qr.h"


class ofxKinectProjectorToolkit {

public:
    
    ofxKinectProjectorToolkit();
    
    void calibrate(vector<ofVec3f> pairsKinect, vector<ofVec2f> pairsProjector);
    //void calibrate(vector<glm::vec3> pairsKinect, vector<glm::vec2> pairsProjector);
    
    ofVec2f getProjectedPoint(ofVec3f worldPoint);
    //glm::vec2 getProjectedPoint(glm::vec3 worldPoint);
    
    //vector<ofVec2f> getProjectedContour(vector<ofVec3f> *worldPoints);
    //vector<glm::vec2> getProjectedContour(vector<glm::vec3> *worldPoints);
    
    vector<double> getCalibration();
    
    void loadCalibration(string path);
    void saveCalibration(string path);
    
    bool isCalibrated() {return calibrated;}
    
private:
    
    dlib::matrix<double, 0, 11> A;
    dlib::matrix<double, 0, 1> y;
    dlib::matrix<double, 11, 1> x;
    
    bool calibrated;
    
};
