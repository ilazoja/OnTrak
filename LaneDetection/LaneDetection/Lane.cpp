#include "Lane.hpp"

#include <iostream>
using namespace std;


Lane::Lane(double slope, double b)
{
	totalSlope += slope;
	totalB += b;
	numberOfLanes++;
}

Lane::Lane()
{

}

Lane::~Lane()
{

}

double Lane::getSlope()
{
	return totalSlope / numberOfLanes;
}

double Lane::getB()
{
	return totalB / numberOfLanes;
}

double Lane::getX(double y)
{
	// y = mx + b
	// (y - b)/m
	if (getSlope() != std::numeric_limits<double>::infinity()) return (y - getSlope() * getB()) / getSlope();
	else return getB();
}

void Lane::addLane(double slope, double b)
{
	totalSlope += slope;
	totalB += b;
	numberOfLanes++;
}