#include "Lane.hpp"
#include <limits>
#include <iostream>
using namespace std;

extern "C"
JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_NativeClass_Lane(JNIEnv *env, jclass type,
                                                           jlong matAddrRgba, jlong matAddrGray);


Lane::Lane(double slope, double b)
{
    totalB += b;
    numberOfLanes++;
    lastFrame = false;
    if (slope > 0) numberOfPosSlope++;
    totalSlope += abs(slope);
}

Lane::Lane()
{

}

Lane::~Lane()
{

}

double Lane::getSlope(bool getCurrent)
{
    if (!lastFrame || getCurrent)
    {
        int factor = 1;
        if (numberOfLanes - numberOfPosSlope > double(numberOfLanes)/2) factor = -1;
        return factor * totalSlope / numberOfLanes;
    }
    else return oldSlope;
}

double Lane::getB(bool getCurrent)
{
    if (!lastFrame || getCurrent) return totalB / numberOfLanes;
    else return oldB;
}

double Lane::getX(double y, bool getCurrent)
{
    // y = mx + b
    // (y - b)/m
    return (getSlope(getCurrent) * getB(getCurrent) + y) / getSlope(getCurrent);
}

void Lane::addLane(double slope, double b, int y1, int y2, int x1, int x2, int rows, int cols)
{
    if (slope > 0) numberOfPosSlope++;
    totalSlope += abs(slope);
    totalB += b;
    numberOfLanes++;
}




