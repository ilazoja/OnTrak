#include "Lane.hpp"

#include <iostream>
using namespace std;


Lane::Lane(double slope, double b)
{
	totalSlope += slope;
	totalB += b;
	numberOfLanes++;
	lastFrame = false;
	if (slope > 0) numberOfPosSlope++;
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
		//if (numberOfLanes - numberOfPosSlope > 0) factor = -1;
		return factor * totalSlope / numberOfLanes;
	}
	else return oldSlope;
}

double Lane::getB(bool getCurrent)
{
	if (!lastFrame || getCurrent) return totalB / numberOfLanes;
	else return oldB;
}

double Lane::getX(double y)
{
	// y = mx + b
	// (y - b)/m
	if (getSlope() != std::numeric_limits<double>::infinity()) return (getSlope() * getB() + y) / getSlope();
	else return getB();
}

void Lane::addLane(double slope, double b)
{
	totalSlope += slope;
	totalB += b;
	numberOfLanes++;
}