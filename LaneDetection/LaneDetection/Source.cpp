// adapted from https://github.com/galenballew/SDC-Lane-and-Vehicle-Detection-Tracking/blob/master/Part%20I%20-%20Simple%20Lane%20Detection/P1.ipynb
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0

#include <windows.h>
#include <iostream>
#include <numeric>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

//default capture width and height
const double pi = 3.1415926535897;
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

void draw_lines(Mat img, vector<Vec4i> lines, Scalar color = Scalar(255, 0, 0), int thickness = 5)
{
	/* workflow
	1) examine each individual line returned by hough & determine if it's in left or right lane by its slope
	   because we are working "upside down" with the array, the left lane will have a negative slope and right positive
	2) track extrema
	3) compute average
	4) solve for b intercept
	5) use extreme to solve for points
	6) smooth frames and cache
	*/

	static vector<double> cache = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static bool first_frame = true;

	int y_global_min = img.rows;
	int y_max = img.rows;

	vector<double> l_slope = {};
	vector<double> r_slope = {};
	vector<Vec4i> l_lane = {};
	vector<Vec4i> r_lane = {};

	double det_slope = 0.4;
	double a = 0.2;

	for (int i = 0; i < lines.size(); i++)
	{
		//1)
		Vec4i l = lines[i];
		double slope = (double(l[3]) - double(l[1])) / (double(l[2]) - double(l[0])); // (y2-y1)/(x2-x1)
		if (slope > det_slope)
		{
			r_slope.push_back(slope);
			r_lane.push_back(l);
		}
		else if (slope < -det_slope)
		{
			l_slope.push_back(slope);
			l_lane.push_back(l);
		}
		
		//2)
		y_global_min = std::min(l[3], y_global_min);
		y_global_min = std::min(l[1], y_global_min);

		line(img, Point(int(l[0]), int(l[1])), Point(int(l[2]), int(l[3])), color, thickness); //enable to display all hough lines
		
	}
	
	if (l_lane.size() == 0 || r_lane.size() == 0)
	{
		return; // no lane detected
	}

	//#3
	double l_slope_mean = 1.0 * std::accumulate(l_slope.begin(), l_slope.end(), 0LL) / l_slope.size();
	double r_slope_mean = 1.0 * std::accumulate(r_slope.begin(), r_slope.end(), 0LL) / r_slope.size();
	vector<double> l_mean = { 0, 0, 0, 0 };
	for (int i = 0; i < l_lane.size(); i++)
	{
		Vec4i l = l_lane[i];
		l_mean[0] += l[0];
		l_mean[1] += l[1];
		l_mean[2] += l[2];
		l_mean[3] += l[3];
	}

	l_mean[0] /= l_lane.size();
	l_mean[1] /= l_lane.size();
	l_mean[2] /= l_lane.size();
	l_mean[3] /= l_lane.size();

	vector<double> r_mean = { 0, 0, 0, 0 };
	for (int i = 0; i < r_lane.size(); i++)
	{
		Vec4i r = r_lane[i];
		r_mean[0] += r[0];
		r_mean[1] += r[1];
		r_mean[2] += r[2];
		r_mean[3] += r[3];
	}

	r_mean[0] /= r_lane.size();
	r_mean[1] /= r_lane.size();
	r_mean[2] /= r_lane.size();
	r_mean[3] /= r_lane.size();

	if (r_slope_mean == 0 || l_slope_mean == 0) return;

	//4) y = mx + b -> b = y - mx
	double l_b = l_mean[1] - l_slope_mean * l_mean[0];
	double r_b = r_mean[1] - r_slope_mean * r_mean[0];

	//5) using y-extrema (2), b intercept (4), and slope (3) solve for x using y = mx + b -> x = (y-b)/m
	// these 4 points are our two lines that we will pass to the draw function
	double l_x1 = (y_global_min - l_b) / l_slope_mean;
	double l_x2 = (y_max - l_b) / l_slope_mean;
	double r_x1 = (y_global_min - r_b) / r_slope_mean;
	double r_x2 = (y_max - r_b) / r_slope_mean;
	
	double l_y1;
	double l_y2;
	double r_y1;
	double r_y2;

	//6
	if (l_x1 > r_x1)
	{
		l_x1 = (l_x1 + r_x1) / 2;
		r_x1 = l_x1;
		l_y1 = (l_slope_mean * l_x1) + l_b;
		r_y1 = (r_slope_mean * r_x1) + r_b;
		l_y2 = (l_slope_mean * l_x2) + l_b;
		r_y2 = (r_slope_mean * r_x2) + r_b;
	}
	else
	{
		l_y1 = y_global_min;
		l_y2 = y_max;
		r_y1 = y_global_min;
		r_y2 = y_max;
	}

	vector<double> current_frame = { l_x1, l_y1, l_x2, l_y2, r_x1, r_y1, r_x2, r_y2 };
	vector<double> next_frame = { 0, 0, 0, 0, 0, 0, 0, 0 };
	if (first_frame)
	{
		next_frame = current_frame;
		first_frame = false;
	}
	else
	{
		for (int i = 0; i < cache.size(); i++)
		{
			next_frame[i] = (1 - a)*cache[i] + a*current_frame[i];
		}
	}

	//line(img, Point(int(next_frame[0]), int(next_frame[1])), Point(int(next_frame[2]), int(next_frame[3])), color, thickness);
	//line(img, Point(int(next_frame[4]), int(next_frame[5])), Point(int(next_frame[6]), int(next_frame[7])), color, thickness);

	imshow("img", img);

	cache = next_frame;
}

int main()
{
	Mat cameraFeed;
	Mat greyFeed;
	Mat hsvFeed;

	Mat mask_yellow;
	Mat mask_white;
	Mat mask_yw;
	Mat mask_yw_image;
	Mat gauss_grey;
	Mat canny_edges;
	vector<Vec4i> hough_lines;
	Mat result;

	VideoCapture capture;
	capture.open("C:\\Ilir\\School\\361\\Videos\\VID_20180225_132717.mp4");
	
	// set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	int number_of_frames = 0;
	
	//Mat gray_image = cvtColor
	while (1)
	{
		if (!(GetKeyState(VK_SPACE) & 1))
		{
			if (GetKeyState('A') & 0x8000/*check if high-order bit is set (1 << 15)*/)
			{
				int rewindFactor = 5;
				if (GetKeyState('Z') & 0x8000) rewindFactor = 10;
				capture.set(CV_CAP_PROP_POS_FRAMES, number_of_frames);
				number_of_frames = number_of_frames - rewindFactor;
				capture >> cameraFeed;
			}
			else if (GetKeyState('S') & 0x8000/*check if high-order bit is set (1 << 15)*/)
			{
				int fastforwardFactor = 5;
				if (GetKeyState('X') & 0x8000) fastforwardFactor = 10;
				capture.set(CV_CAP_PROP_POS_FRAMES, number_of_frames);
				number_of_frames = number_of_frames + fastforwardFactor;
				capture >> cameraFeed;
			}
			else if (GetKeyState('E') & 0x8000)
			{
				break;
			}
			else
			{
				//store image to matrix
				if (!capture.read(cameraFeed))
				{
					// reset if at the end
					capture.set(CV_CAP_PROP_POS_FRAMES, 0);
					capture.read(cameraFeed);
				}
				number_of_frames = capture.get(CV_CAP_PROP_POS_FRAMES);
			}
		}

		// get greyscale image
		cvtColor(cameraFeed, greyFeed, CV_BGR2GRAY);

		// get HSV
		cvtColor(cameraFeed, hsvFeed, CV_BGR2HSV);

		inRange(hsvFeed, Scalar(20, 100, 100), Scalar(30, 255, 255), mask_yellow); // get yellow mask (if any yellow in track)
		inRange(greyFeed, 200, 255, mask_white); // get white mask
		bitwise_or(mask_white, mask_yellow, mask_yw); // get white and yellow mask combined
		bitwise_and(greyFeed, mask_yw, mask_yw_image); // get filtered image

		GaussianBlur(mask_white, gauss_grey, Size(5, 5), 11); // helps suppress noise

		double low_threshold = 50;
		double high_threshold = 150;
		Canny(gauss_grey, canny_edges, low_threshold, high_threshold); // canny edge detection

		// rho and theta are the distance and angular resolution of the grid in Hough space
		double rho = 1;
		double theta =  pi / 180;
		// threshold is minimum number of intersections in a grid for candidate line to go to output
		int threshold = 30;
		double min_line_len = 500;
		double max_line_gap = 50;
		HoughLinesP(gauss_grey, hough_lines, rho, theta, threshold, min_line_len, max_line_gap);
		Mat line_img(gauss_grey.rows, gauss_grey.cols, CV_8UC3, Scalar(0,0,0));
		draw_lines(line_img, hough_lines);

		addWeighted(line_img, 0.8, cameraFeed, 1, 0, result);

		imshow("Raw", gauss_grey);
		imshow("Result", result);

		
		waitKey(30);
	}
}