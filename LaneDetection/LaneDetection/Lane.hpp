#include <jni.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
#ifndef ONTRAKDEMOC_CONVERT_GRAY_H
#define ONTRAKDEMOC_CONVERT_GRAY_H
extern "C" {

#endif //ONTRAKDEMOC_CONVERT_GRAY_H
int toGray(Mat img, Mat &gray);
JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_NativeClass_Lane(JNIEnv *env, jclass type,
														   jlong matAddrRgba, jlong matAddrGray);

#pragma once

class Lane {
public:
	Lane(double slope, double b);

	Lane();

	~Lane();

	double getSlope(bool getCurrent = false);

	double getB(bool getCurrent = false);

	double getX(double y, bool getCurrent = false);

	void addLane(double slope, double b, int y1, int y2, int x1, int x2, int rows, int cols);
    double totalSlope = 0;
    double totalB = 0;
    int numberOfLanes = 0;
    int numberOfPosSlope = 0;

    double oldSlope = 0;
    double oldB = 0;
    bool lastFrame = false;

    bool right;
    bool lastRight;
    bool isFullLine = false;
    int buffer = 2;
    bool isLaneLine = true;

};
}