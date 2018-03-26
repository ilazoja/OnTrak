#include "Lane.hpp"
#include <limits>
#include <iostream>
using namespace std;

extern "C"
JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_NativeClass_Lane(JNIEnv *env, jclass type,
                                                           jlong matAddrRgba, jlong matAddrGray);


    Lane::Lane(double slope, double b) {
        totalSlope += slope;
        totalB += b;
        numberOfLanes++;
        lastFrame = false;
        if (slope > 0) numberOfPosSlope++;
    }

    Lane::Lane() {

    }

    Lane::~Lane() {

    }

    double Lane::getSlope(bool getCurrent) {
        if (!lastFrame || getCurrent) {
            int factor = 1;
            //if (numberOfLanes - numberOfPosSlope > 0) factor = -1;
            return factor * totalSlope / numberOfLanes;
        } else return oldSlope;
    }

    double Lane::getB(bool getCurrent) {
        if (!lastFrame || getCurrent) return totalB / numberOfLanes;
        else return oldB;
    }

    double Lane::getX(double y, bool getCurrent) {
        // y = mx + b
        // (y - b)/m
        if (getSlope(getCurrent) != std::numeric_limits<double>::infinity())
            return (getSlope(getCurrent) * getB(getCurrent) + y) / getSlope(getCurrent);
        else return getB(getCurrent);
    }

    void Lane::addLane(double slope, double b, int y1, int y2, int x1, int x2, int rows, int cols) {
        totalSlope += slope;
        totalB += b;
        numberOfLanes++;
        if (isLaneLine) {
            if (y1 > rows - 300 || y2 > rows - 300 || x1 > cols - 300 || x2 > cols - 300 ||
                x1 < 300 || x2 < 300)
                isFullLine = true;
        }
    }




