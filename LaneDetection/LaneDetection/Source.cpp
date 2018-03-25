// adapted from https://github.com/galenballew/SDC-Lane-and-Vehicle-Detection-Tracking/blob/master/Part%20I%20-%20Simple%20Lane%20Detection/P1.ipynb
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0

// calculate intersection to figure out middle line?
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
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 144;
int H_MAX = 256;
int L_MIN = 42; //0 //121
int L_MAX = 256;
int S_MIN = 39; //96 //0
int S_MAX = 256;

bool mouseIsDragging;//used for showing a rectangle on screen as user clicks and drags mouse
bool mouseMove;
bool rectangleSelected;
cv::Point initialClickPoint, currentMousePoint; //keep track of initial point clicked and current position of mouse
cv::Rect rectangleROI; //this is the ROI that the user has selected
vector<int> H_ROI, L_ROI, S_ROI;// HSV values from the click/drag ROI region stored in separate vectors so that we can sort them easily

RNG rng(12345);

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed


}

bool laneIsEmpty(Lane lane)
{
	if (lane.numberOfLanes < 1) return true;
	else return false;
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

void createTrackbars() {
	//create window for trackbars
	String trackbarWindowName = "Trackbars";

	namedWindow(trackbarWindowName, 0);
	//create memory to store trackbar name on window
	char TrackbarName[50];
	sprintf_s(TrackbarName, "H_MIN", H_MIN);
	sprintf_s(TrackbarName, "H_MAX", H_MAX);
	sprintf_s(TrackbarName, "L_MIN", L_MIN);
	sprintf_s(TrackbarName, "L_MAX", L_MAX);
	sprintf_s(TrackbarName, "S_MIN", S_MIN);
	sprintf_s(TrackbarName, "S_MAX", S_MAX);
	//create trackbars and insert them into window
	//3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
	//the max value the trackbar can move (eg. H_HIGH), 
	//and the function that is called whenever the trackbar is moved(eg. on_trackbar)
	//                                  ---->    ---->     ---->      
	createTrackbar("H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar);
	createTrackbar("L_MIN", trackbarWindowName, &L_MIN, L_MAX, on_trackbar);
	createTrackbar("L_MAX", trackbarWindowName, &L_MAX, L_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar);

}

void clickAndDrag_Rectangle(int event, int x, int y, int flags, void* param) {
	//only if calibration mode is true will we use the mouse to change HSV values
	
	bool calibrationMode = true;//used for showing debugging windows, trackbars etc.


	if (calibrationMode == true) {
		//get handle to video feed passed in as "param" and cast as Mat pointer
		Mat* videoFeed = (Mat*)param;

		if (event == CV_EVENT_LBUTTONDOWN && mouseIsDragging == false)
		{
			//keep track of initial point clicked
			initialClickPoint = cv::Point(x, y);
			//user has begun dragging the mouse
			mouseIsDragging = true;
		}
		/* user is dragging the mouse */
		if (event == CV_EVENT_MOUSEMOVE && mouseIsDragging == true)
		{
			//keep track of current mouse point
			currentMousePoint = cv::Point(x, y);
			//user has moved the mouse while clicking and dragging
			mouseMove = true;
		}
		/* user has released left button */
		if (event == CV_EVENT_LBUTTONUP && mouseIsDragging == true)
		{
			//set rectangle ROI to the rectangle that the user has selected
			rectangleROI = Rect(initialClickPoint, currentMousePoint);

			//reset boolean variables
			mouseIsDragging = false;
			mouseMove = false;
			rectangleSelected = true;
		}

		if (event == CV_EVENT_RBUTTONDOWN) {
			//user has clicked right mouse button
			//Reset HSV Values
			H_MIN = 0;
			L_MIN = 0;
			S_MIN = 0;
			H_MAX = 255;
			L_MAX = 255;
			S_MAX = 255;

		}
		if (event == CV_EVENT_MBUTTONDOWN) {

			//user has clicked middle mouse button
			//enter code here if needed.
		}
	}

}

void recordHSV_Values(cv::Mat frame, cv::Mat hsv_frame) {

	//save HSV values for ROI that user selected to a vector
	if (mouseMove == false && rectangleSelected == true) {

		//clear previous vector values
		if (H_ROI.size()>0) H_ROI.clear();
		if (L_ROI.size()>0) L_ROI.clear();
		if (S_ROI.size()>0) S_ROI.clear();
		//if the rectangle has no width or height (user has only dragged a line) then we don't try to iterate over the width or height
		if (rectangleROI.width<1 || rectangleROI.height<1) cout << "Please drag a rectangle, not a line" << endl;
		else {
			for (int i = rectangleROI.x; i<rectangleROI.x + rectangleROI.width; i++) {
				//iterate through both x and y direction and save HSV values at each and every point
				for (int j = rectangleROI.y; j<rectangleROI.y + rectangleROI.height; j++) {
					//save HSV value at this point
					H_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[0]);
					L_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[1]);
					S_ROI.push_back((int)hsv_frame.at<cv::Vec3b>(j, i)[2]);
				}
			}
		}
		//reset rectangleSelected so user can select another region if necessary
		rectangleSelected = false;
		//set min and max HSV values from min and max elements of each array

		if (H_ROI.size()>0) {
			//NOTE: min_element and max_element return iterators so we must dereference them with "*"
			H_MIN = *std::min_element(H_ROI.begin(), H_ROI.end());
			H_MAX = *std::max_element(H_ROI.begin(), H_ROI.end());
			cout << "MIN 'H' VALUE: " << H_MIN << endl;
			cout << "MAX 'H' VALUE: " << H_MAX << endl;
		}
		if (L_ROI.size()>0) {
			L_MIN = *std::min_element(L_ROI.begin(), L_ROI.end());
			L_MAX = *std::max_element(L_ROI.begin(), L_ROI.end());
			cout << "MIN 'L' VALUE: " << L_MIN << endl;
			cout << "MAX 'L' VALUE: " << L_MAX << endl;
		}
		if (S_ROI.size()>0) {
			S_MIN = *std::min_element(S_ROI.begin(), S_ROI.end());
			S_MAX = *std::max_element(S_ROI.begin(), S_ROI.end());
			cout << "MIN 'S' VALUE: " << S_MIN << endl;
			cout << "MAX 'S' VALUE: " << S_MAX << endl;
		}

	}

	if (mouseMove == true) {
		//if the mouse is held down, we will draw the click and dragged rectangle to the screen
		rectangle(frame, initialClickPoint, cv::Point(currentMousePoint.x, currentMousePoint.y), cv::Scalar(0, 255, 0), 1, 8, 0);
	}


}

void update_line(Mat img, int lowerXBound, int lowerYBound, Mat& line_img, vector<Lane>& lanes)
{
	Mat greyFeed;
	Mat hsvFeed;

	Mat mask_hls;
	Mat mask_white;
	Mat mask_whls;
	Mat mask_whls_image;
	Mat gaussFeed;
	Mat canny_edges;
	vector<Vec4i> hough_lines;
	Mat result;

	GaussianBlur(img, gaussFeed, Size(23, 23), 100); // helps suppress noise

												  // get greyscale image

	cvtColor(gaussFeed, greyFeed, CV_BGR2GRAY);

	mask_hls = hlsSelect(gaussFeed, Scalar(0, 117, 0), Scalar(60, 256, 256));
	inRange(greyFeed, 200, 255, mask_white); // get white mask
	bitwise_or(mask_white, mask_hls, mask_whls); // get white and yellow mask combined
	bitwise_and(greyFeed, mask_whls, mask_whls_image); // get filtered image

	double low_threshold = 100;
	double high_threshold = low_threshold * 3;
	Canny(mask_whls_image, canny_edges, low_threshold, high_threshold); // canny edge detection

																		// rho and theta are the distance and angular resolution of the grid in Hough space
	double rho = 1;
	double theta = pi / 180;
	// threshold is minimum number of intersections in a grid for candidate line to go to output
	int thresholdHough = 100;
	double min_line_len = 200;
	double max_line_gap = 200;
	HoughLinesP(canny_edges, hough_lines, rho, theta, thresholdHough, min_line_len, max_line_gap);

	
	imshow("canny", mask_whls);
	//int decision = process_lines(line_img, hough_lines);

	// process_lines

	/* workflow
	1) examine each individual line returned by hough & determine if it's in left or right lane by its slope
	because we are working "upside down" with the array, the left lane will have a negative slope and right positive
	2) track extrema
	3) compute average
	4) solve for b intercept
	5) use extreme to solve for points
	6) smooth frames and cache
	*/

	int y_global_min = line_img.rows;
	int y_max = line_img.rows;

	double det_slope = 0.5;
	double a = 0.4;

	for (int i = 0; i < hough_lines.size(); i++)
	{
		//1)
		Vec4i l = hough_lines[i];
		l[0] = l[0] + lowerXBound;
		l[1] = l[1] + lowerYBound;
		l[2] = l[2] + lowerXBound;
		l[3] = l[3] + lowerYBound;
		if (l[2] - l[0] == 0) l[2] = l[2] + 1;
		double slope = (double(l[3]) - double(l[1])) / (double(l[2]) - double(l[0])); // (y2-y1)/(x2-x1)
		double b = 0;

		//slope = std::abs(slope);
		//b = std::abs(b);
		int minIndex = -1;
		if (abs(slope) > det_slope)
		{
			// MED classifier

			b = (slope * double(l[0]) - double(l[1])) / slope;
			double x = (slope * b + line_img.rows) / slope;
			double minDist = 65000;

			for (int j = 0; j < lanes.size(); j++)
			{
				Lane lane = lanes[j];
				double lx = lane.getX(line_img.rows);
				double lB = lane.getB();
				double dist = abs(pow(x, 2) - 2 * x*lx + pow(lx, 2) + pow(b, 2) - 2 * b*lB + pow(lB, 2));
				if (dist < minDist)
				{
					minDist = dist;
					minIndex = j;
				}
			}

			if (minIndex >= 0) lanes[minIndex].addLane(slope, b, l[3], l[1], l[0], l[2], line_img.rows, line_img.cols);
			else
			{
				Lane lane = Lane(slope, b);
				lanes.push_back(lane);
			}
		}
		line(line_img, Point(int(l[0]), int(l[1])), Point(int(l[2]), int(l[3])), Scalar(255, 0, 0), 5); //enable to display all hough lines
	}

}

bool find_obstacles(Mat img)
{
	Mat greyFeed;
	Mat hsvFeed;

	Mat mask_hls;
	Mat mask_white;
	Mat mask_whls;
	Mat mask_whls_image;
	Mat gaussFeed;
	Mat canny_edges;
	vector<Vec4i> hough_lines;
	Mat result;
	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;

	GaussianBlur(img, gaussFeed, Size(23, 23), 100); // helps suppress noise

													 // get greyscale image

	cvtColor(gaussFeed, greyFeed, CV_BGR2GRAY);

	mask_hls = hlsSelect(gaussFeed, Scalar(H_MIN, L_MIN, S_MIN), Scalar(H_MAX, L_MAX, S_MAX));
	inRange(greyFeed, 200, 255, mask_white); // get white mask
	bitwise_not(mask_white, mask_white);
	bitwise_and(mask_white, mask_hls, mask_whls); // get white and yellow mask combined
	bitwise_and(greyFeed, mask_whls, mask_whls_image); // get filtered image

	double low_threshold = 100;
	double high_threshold = low_threshold * 3;
	Canny(mask_whls_image, canny_edges, low_threshold, high_threshold); // canny edge detection

	//findContours(canny_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Draw contours
	//Mat drawing = Mat::zeros(canny_edges.size(), CV_8UC3);
	//for (int i = 0; i< contours.size(); i++)
	//{
	//	Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	drawContours(drawing, contours, i, color, 2, 8, hierarchy, 0, Point());
	//}
	vector<Point> white_pixels;
	findNonZero(canny_edges, white_pixels);
	int threshold = 400;
	imshow("obstacle", canny_edges);
	return white_pixels.size() > threshold;
}

int main()
{
	bool detectObjects = true;


	Mat cameraFeed;
	Mat result;
	Mat line_img;

	bool started = false;
	int linesCrossed = 0;

	int searchRange = 200;
	bool obstacleDetectedLastFrame = false;
	bool obstacleDetectedCurrentFrame = false;

	VideoCapture capture;
	capture.open("C:\\Ilir\\School\\361\\Videos\\VID_20180225_132827.mp4");
	
	// set height and width of capture frame
	capture.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	capture.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

	int number_of_frames = 0;
	createTrackbars();
	int decision = 0;
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

		static vector<Lane> lanes = {};
		
		Mat line_img(cameraFeed.rows, cameraFeed.cols, CV_8UC3, Scalar(0, 0, 0));

		if (number_of_frames % 5 == 0)
		{
			if (detectObjects)
			{
				obstacleDetectedLastFrame = obstacleDetectedCurrentFrame;
				obstacleDetectedCurrentFrame = find_obstacles(cameraFeed(Range(0, line_img.rows / 2), Range(line_img.cols / 4, line_img.cols * 3 / 4)));
				if (obstacleDetectedCurrentFrame && !obstacleDetectedLastFrame) linesCrossed--;
				if (!obstacleDetectedCurrentFrame && obstacleDetectedLastFrame) linesCrossed++;
			}
			
			if (!started || lanes.size() == 0)
			{
				Mat sub_img = cameraFeed(Range(0, line_img.rows), Range(0, line_img.cols));
				update_line(sub_img, 0, 0, line_img, lanes);
			}
			else
			{
				bool lookedAtLeftBound = false;
				bool lookedAtRightBound = false; 
				int lowerCheckBound = line_img.cols - searchRange;
				int higherCheckBound = searchRange;
				for (int j = 0; j < lanes.size(); j++)
				{
					Lane& lane = lanes[j];
					int lowerBound;
					int higherBound;
					
					if (lane.getSlope() < 0)
					{
						lowerBound = lane.getX(line_img.rows) - searchRange;
						if (lowerBound < 0 || lowerBound < line_img.cols)
						{
							lowerBound = 0;
							if (lane.getX(line_img.rows) > 0)
							{
								higherCheckBound = std::min((int)lane.getX(line_img.rows), higherCheckBound);
								lookedAtLeftBound = true;
							}
						    
						}
						higherBound = (int)lane.getX(line_img.rows / 2) + searchRange;
						if (higherBound > line_img.cols || higherBound < 0)
						{
							higherBound = line_img.cols;
							if (lane.getX(line_img.rows / 2) < line_img.cols)
							{
								lowerCheckBound = std::max((int)lane.getX(line_img.rows / 2), lowerCheckBound);
								lookedAtRightBound = true;
							}
						}
					}
					else if (lane.getSlope() > 0)
					{
						lowerBound = (int)lane.getX(line_img.rows / 2) - searchRange;
						if (lowerBound < 0 || lowerBound < line_img.cols)
						{
							lowerBound = 0;
							if (lane.getX(line_img.rows / 2) > 0)
							{
								higherCheckBound = std::min((int)lane.getX(line_img.rows / 2), higherCheckBound);
								lookedAtLeftBound = true;
							}
						}
						higherBound = lane.getX(line_img.rows) + searchRange;
						if (higherBound > line_img.cols || higherBound < 0)
						{
							higherBound = line_img.cols;
							if (lane.getX(line_img.rows) < line_img.cols) {
								lowerCheckBound = std::max((int)lane.getX(line_img.rows), lowerCheckBound);
								lookedAtRightBound = true;
							}
						}
					}

					Mat sub_img = cameraFeed(Range(line_img.rows/2, line_img.rows), Range(lowerBound, higherBound));
					update_line(sub_img, lowerBound, line_img.rows/2, line_img, lanes);

				}
				if (!lookedAtLeftBound)
				{
					Mat sub_img = cameraFeed(Range(line_img.rows / 2, line_img.rows), Range(0, higherCheckBound));
					update_line(sub_img, 0, line_img.rows / 2, line_img, lanes);
				}
				if (!lookedAtRightBound)
				{
					Mat sub_img = cameraFeed(Range(line_img.rows / 2, line_img.rows), Range(lowerCheckBound, line_img.cols));
					update_line(sub_img, line_img.cols - searchRange, line_img.rows / 2, line_img, lanes);
				}
			}
			
			if (lanes.size() > 1) started = true;
			/*if (l_lane.size() == 0 || r_lane.size() == 0)
			{
			return -1; // no lane detected
			}*/


			double y1 = 0;
			double y2 = cameraFeed.rows;
			double y3 = cameraFeed.rows * 1000;

			// compute distance

			// get middle x of the screen
			double xpoint = (double)cameraFeed.cols / 2;

			//Lane* l_lane = NULL;
			//Lane* r_lane = NULL;

			double threshold = 150;

			double distl = 1000000;
			double distr = -1000000;
			int leftLaneIndex = -1;
			int rightLaneIndex = -1;

			int rightLaneClosestIndex = -1;
			int leftLaneClosestIndex = -1;
			double distlFromUser = line_img.cols;
			double distrFromUser = -line_img.cols;

			lanes.erase(std::remove_if(lanes.begin(), lanes.end(), laneIsEmpty), lanes.end());
			for (int i = 0; i < lanes.size(); i++)
			{
				Lane& lane = lanes[i];
				double x1 = lane.getX(y1, true);
				double x2 = lane.getX(y2, true);
				double x3 = lane.getX(y3, true);

				//if (lane.isLaneLine || x2 >= 0 || x2 <= cameraFeed.cols)
				//{
				//	if (!lane.isFullLine)
				//	{
				//		--lane.buffer;
				//	}
				//	else lane.buffer = 2;
				//	if (lane.buffer == 0)
				//	{
				//		lane.isLaneLine = false;
				//	}
				//}
				if (lane.isLaneLine)
				{
					
					line(line_img, Point(int(x1), int(y1)), Point(int(x2), int(y2)), Scalar(0, 255, 0), 5);

					// get distance
					double distExtend = xpoint - x3;
					double dist = xpoint - x2;

					if (lane.lastFrame) lane.lastRight = lane.right;
					if (distExtend > 0)
					{
						lane.right = false;
						if (distExtend < distl) {
							distl = distExtend;
							leftLaneIndex = i;
						}
					}
					else if (distExtend < 0)
					{
						lane.right = true;
						if (distExtend > distr) {
							distr = distExtend;
							rightLaneIndex = i;
						}
					}
					if (dist > 0)
					{
						if (dist < distlFromUser) {
							distlFromUser = dist;
							leftLaneClosestIndex = i;

						}
					}
					else if (dist < 0)
					{
						if (dist > distrFromUser) {
							distrFromUser = dist;
							rightLaneClosestIndex = i;
						}
					}
					if (lane.lastFrame)
					{
						if (lane.lastRight && !lane.right) linesCrossed++;
						else if (!lane.lastRight && lane.right) linesCrossed--;
					}

				}

				lane.oldSlope = lane.getSlope(true);
				lane.oldB = lane.getB(true);
				lane.totalSlope = 0;
				lane.totalB = 0;
				lane.numberOfLanes = 0;
				lane.numberOfPosSlope = 0;
				lane.isFullLine = false;
				lane.lastFrame = true;

			}
			if (leftLaneClosestIndex != leftLaneIndex && leftLaneClosestIndex > -1 && leftLaneIndex > -1)
			{
				distlFromUser = (double)cameraFeed.cols / 2 - lanes[leftLaneIndex].getX(line_img.rows, true);
				lanes[leftLaneClosestIndex].isLaneLine = false;
			}
			if (rightLaneClosestIndex != rightLaneIndex && rightLaneClosestIndex > -1 && rightLaneIndex > -1)
			{
				distrFromUser = (double)cameraFeed.cols / 2 - lanes[rightLaneIndex].getX(line_img.rows, true);
				lanes[rightLaneClosestIndex].isLaneLine = false;
			}
			if (!started) decision = 3;
			else if (linesCrossed < 0) decision = 2;
			else if (linesCrossed > 0) decision = 1;
			else if (distlFromUser < threshold) decision = 1;
			else if (distrFromUser > -threshold) decision = 2;
			else decision = 0;

			//--------------------

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
		
	}

	return 0;
}