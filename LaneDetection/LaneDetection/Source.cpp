// adapted from https://github.com/galenballew/SDC-Lane-and-Vehicle-Detection-Tracking/blob/master/Part%20I%20-%20Simple%20Lane%20Detection/P1.ipynb
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0

#include <windows.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

//default capture width and height
const double pi = 3.1415926535897;
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

void draw_lines(Mat img, Mat lines, Scalar color = Scalar(255, 0, 0), int thickness = 0)
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

	static int cache[8];
	static int first_frame[8];
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
	Mat hough_lines;

	VideoCapture capture;
	capture.open("C:\\Ilir\\School\\361\\Videos\\VID_20180225_132450.mp4");
	
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

		GaussianBlur(mask_yw_image, gauss_grey, Size(5, 5), 11); // helps suppress noise

		double low_threshold = 50;
		double high_threshold = 150;
		Canny(gauss_grey, canny_edges, low_threshold, high_threshold); // canny edge detection

		// rho and theta are the distance and angular resolution of the grid in Hough space
		double rho = 4;
		double theta =  pi / 180;
		// threshold is minimum number of intersections in a grid for candidate line to go to output
		int threshold = 30;
		double min_line_len = 100;
		double max_line_gap = 180;
		HoughLinesP(gauss_grey, hough_lines, rho, theta, threshold, min_line_len, max_line_gap);

		imshow("Raw", cameraFeed);
		imshow("Hough", hough_lines);

		
		waitKey(30);
	}
}