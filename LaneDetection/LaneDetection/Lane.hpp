#pragma once

class Lane
{
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