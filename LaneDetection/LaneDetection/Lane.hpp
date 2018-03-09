#pragma once

class Lane
{
public:
	Lane(double slope, double b);
	Lane();
	~Lane();

	double getSlope();
	double getB();
	double getX(double y);
	void addLane(double slope, double b);

	double totalSlope = 0;
	double totalB = 0;
	int numberOfLanes = 0;

	double oldSlope = 0;
	double oldB = 0;
};