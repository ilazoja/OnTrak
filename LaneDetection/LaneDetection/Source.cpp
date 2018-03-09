// adapted from https://github.com/galenballew/SDC-Lane-and-Vehicle-Detection-Tracking/blob/master/Part%20I%20-%20Simple%20Lane%20Detection/P1.ipynb
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0

// maybe change left or right classification based on location from middle?

#include <windows.h>
#include <iostream>
#include <numeric>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "Lane.hpp"

using namespace std;
using namespace cv;

//default capture width and height
const double pi = 3.1415926535897;
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

int process_lines(Mat img, vector<Vec4i> lines, Scalar color = Scalar(255, 0, 0), int thickness = 5)
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
	static int linesCrossed = 0;
	static double lastDistl = 0;
	static double lastDistr = 0;

	int y_global_min = img.rows;
	int y_max = img.rows;

	vector<double> l_slope = {};
	vector<double> r_slope = {};
	vector<Vec4i> l_lane = {};
	vector<Vec4i> r_lane = {};

	static vector<Lane> lanes = {};

	double det_slope = 1;
	double a = 0.4;

	for (int i = 0; i < lines.size(); i++)
	{
		//1)
		Vec4i l = lines[i];
		double slope = (double(l[3]) - double(l[1])) / (double(l[2]) - double(l[0])); // (y2-y1)/(x2-x1)
		double b = 0;

		//slope = std::abs(slope);
		//b = std::abs(b);
		int minIndex = -1;
		if (abs(slope) > det_slope)
		{

			// MED classifier
			if (slope != std::numeric_limits<double>::infinity())
			{
				b = (double(l[1]) - slope * double(l[0])) / slope;
				double minDist = 10000;
				for (int j = 0; j < lanes.size(); j++)
				{
					Lane lane = lanes[j];
					double lSlope = lane.getSlope();
					double lB = lane.getB();
					double dist = pow(slope, 2) - 2 * slope*lSlope + pow(lSlope, 2) + pow(b, 2) - 2 * b*lB + pow(lB, 2);
					if (dist < minDist)
					{
						minDist = dist;
						minIndex = j;
					}
				}
			}
			else
			{
				b = l[0];
				double minDist = 100;
				for (int j = 0; j < lanes.size(); j++)
				{
					Lane lane = lanes[j];
					double lB = lane.getB();
					double dist = abs(lB - b);
					if (dist < minDist)
					{
						minDist = dist;
						minIndex = j;
					}
				}
			}


			if (minIndex >= 0) lanes[minIndex].addLane(slope, b);
			else
			{
				Lane lane = Lane(slope, b);
				lanes.push_back(lane);
			}
		}
		line(img, Point(int(l[0]), int(l[1])), Point(int(l[2]), int(l[3])), color, thickness); //enable to display all hough lines
		
	}
	
	/*if (l_lane.size() == 0 || r_lane.size() == 0)
	{
		return -1; // no lane detected
	}*/
	
	
	double y1 = 0;
	double y2 = img.rows;

	// compute distance

	// get middle x of the screen
	double xpoint = (double)img.cols / 2;

	//Lane* l_lane = NULL;
	//Lane* r_lane = NULL;

	double threshold = 100;

	double distl = threshold * 2;
	double distr = -threshold * 2;
	for (int i = 0; i < lanes.size(); i++)
	{
		Lane lane = lanes[i];
		double x1 = lane.getX(y1);
		double x2 = lane.getX(y2);
		line(img, Point(int(x1), int(y1)), Point(int(x2), int(y2)), Scalar(0, 255, 0), thickness);

		// get distance
		double dist = xpoint - x2;

		if (dist > 0 && dist < distl) distl = dist;
		else if (dist < 0 && dist > distr) distr = dist;


	}
	
	//cache = next_frame;


	if (lastDistl > 0 && distl < 0) {
		linesCrossed--;
	}
	if (distl < threshold || linesCrossed < 0)
	{
		lastDistl = distl;
		return 1;	
	}
	if (lastDistr < 0 && distr > 0) {
		linesCrossed++;
	}
	if (distr > -threshold || linesCrossed > 0) {
		lastDistr = distr;
		return 2;
	}
	return 0;
}

// Sobel gradients - Canny edeg detection combines the sobel gradient for both x and y. By breaking it apart into its compoennts, we can produce a refined version of Canny edge detection
Mat absSobelThresh(Mat img, char orient = 'x', int threshLow = 0, int threshHigh = 255)
{
	Mat grey;
	Mat scaled_abs_sobel;
	// Convert to grayscale
	cvtColor(img, grey, COLOR_RGB2GRAY);

	//Apply x or y gradient with the OpenCV Sobel() function and take the absolute value
	if (orient == 'x') 
	{
		Mat sobel;
		Sobel(grey, sobel, CV_64F, 1, 0);
		convertScaleAbs(sobel, scaled_abs_sobel);
	}
	else if (orient == 'y')
	{
		Mat sobel;
		Sobel(grey, sobel, CV_64F, 0, 1);
		convertScaleAbs(sobel, scaled_abs_sobel); // gets abs and rescale to 8 uint
	}

	// apply the threshold
	Mat binary_output;
	inRange(scaled_abs_sobel, threshLow, threshHigh, binary_output);
	return binary_output;

}

// Gradient magnitude - This function will filter based on min/max magnitude for the gradient. This function looks at the combined xy gradient
// but also could be altered to filter on the magnitude in a single direction, or some linear combination of the two
Mat magThreshold(Mat img, int sobel_kernel = 3, int threshLow = 0, int threshHigh = 255)
{
	Mat grey;
	Mat x;
	Mat y;
	Mat abs_x;
	Mat abs_y;
	Mat grad;
	Mat binary_output;

	// 1) Convert to grayscale
	cvtColor(img, grey, COLOR_RGB2GRAY);

	// 2) Take the gradient in x and y separately
	Sobel(grey, x, CV_64F, 1, 0, sobel_kernel);
	Sobel(grey, y, CV_64F, 0, 1, sobel_kernel);

	// 3) Gets abs and scale to uint8
	convertScaleAbs(x, abs_x);
	convertScaleAbs(y, abs_y);

	// 4) Approximate gradient by adding both directional gradients
	addWeighted(abs_x, 0.5, abs_y, 0.5, 0, grad);

	// 5) apply threshold
	inRange(grad, threshLow, threshHigh, binary_output);

	return binary_output;
}

// Gradient Direction - This function will filter based on the direction of the gradient. 
// For lane detection, we will be interested in vertical linse that are +/0 some threshold near pi/2
Mat dirThreshold(Mat img, int sobel_kernel = 3, int threshLow = 0, int threshHigh = pi/2)
{
	Mat grey;
	Mat x;
	Mat y;
	Mat abs_x;
	Mat abs_y;
	Mat direction;
	Mat binary_output;

	// 1) Convert to grayscale
	cvtColor(img, grey, COLOR_RGB2GRAY);

	// 2) Take the gradient in x and y separately
	Sobel(grey, x, CV_64F, 1, 0, sobel_kernel);
	Sobel(grey, y, CV_64F, 0, 1, sobel_kernel);
	
	// 3) Gets abs
	abs_x = abs(x);
	abs_y = abs(y);

	// 4) calculate direction of gradient
	phase(abs_x, abs_y, direction);

	// 5) threshold direction
	inRange(direction, threshLow, threshHigh, binary_output);

	return binary_output;
}

// Saturation Channel & Red Channel Filters
// The gradient filters above all convert the og image in to greyscale losing a lot of information. Lane lines could be yellow or white and we can use that to try to locate and track them
// The Hue, Saturation, Lightness colour space will help. In particular, the S channel of an HSL image retains a lot of information about lane lines, especially with shadows on the track
// The Red channel of RGB also does a good job of creating binary images of lane lines;
Mat hlsSelect(Mat img, Scalar threshLow = Scalar(0, 0, 0), Scalar threshHigh = Scalar(255, 255, 255))
{
	Mat hls;
	Mat binary_output;

	// 1) Convert to HLS colour space
	cvtColor(img, hls, COLOR_RGB2HLS);

	// 2) Apply thresholds
	inRange(hls, threshLow, threshHigh, binary_output);

	return binary_output;
}

Mat redSelect(Mat img, int lowThresh = 0, int highThresh = 255)
{
	Mat binary_output;
	inRange(img, Scalar(lowThresh, 0, 0), Scalar(highThresh, 255, 255), binary_output);
	return binary_output;
}

/// Combining Filter Methods
// Mix and match different filter methods, each with unique threshold values, to get a refined binary image
Mat binaryPipeline(Mat img)
{
	Mat img_blur;
	GaussianBlur(img, img_blur, Size(9,9), 21);

	// color channels
	Mat s_binary = hlsSelect(img_blur, Scalar(0, 120, 140), Scalar(255, 255, 255));
	// red_binary = redSelect(img_blur, 200, 255);
	imshow("s_binary", s_binary);
	// Sobel x
	Mat x_binary = absSobelThresh(img_blur, 'x', 25, 200);
	Mat y_binary = absSobelThresh(img_blur, 'y', 25, 200);

	// Magnitude & Direction
	Mat mag_binary = magThreshold(img_blur, 3, 30, 100);
	Mat dir_binary = dirThreshold(img_blur, 3, 0.8, 1.2);

	// Stack each channel
	Mat final_binary;
	Mat gradient;
	Mat xy_binary;
	bitwise_and(x_binary, y_binary, xy_binary);
	Mat magdir_binary;
	bitwise_and(mag_binary, dir_binary, magdir_binary);
	bitwise_or(xy_binary, magdir_binary, gradient);
	bitwise_or(s_binary, gradient, final_binary);

	return final_binary;

	
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

		inRange(hsvFeed, Scalar(0, 0, 0), Scalar(255, 255, 255), mask_yellow); // get yellow mask (if any yellow in track)
		inRange(greyFeed, 250, 255, mask_white); // get white mask
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
		int decision = process_lines(line_img, hough_lines);

		addWeighted(line_img, 0.8, cameraFeed, 1, 0, result);
		int font = FONT_HERSHEY_SIMPLEX;
		string msg;
		if (decision == 1) msg = "LEFT";
		else if (decision == 2) msg = "RIGHT";
		else msg = "ON TRACK";
		putText(result, msg, Point(10, 500), font, 4, Scalar(255, 255, 255), 2, LINE_AA);

		imshow("Raw", line_img);
		imshow("Result", result);

		
		waitKey(30);
	}

	return 0;
}