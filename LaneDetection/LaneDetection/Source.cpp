// adapted from https://github.com/galenballew/SDC-Lane-and-Vehicle-Detection-Tracking/blob/master/Part%20I%20-%20Simple%20Lane%20Detection/P1.ipynb
// https://medium.com/@galen.ballew/opencv-lanedetection-419361364fc0

// maybe change left or right classification based on location from middle?

#include <limits>
#include <iostream>
#include <numeric>
#include "opencv2/imgproc.hpp"
#include <opencv2/highgui.hpp>
#include "Lane.hpp"

using namespace std;
using namespace cv;


//default capture width and height
const double pi = 3.1415926535897;
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;
//initial min and max HSV filter values.
//these will be changed using trackbars
int H_MIN = 137;
int H_MAX = 256;
int L_MIN = 0; //0 //121
int L_MAX = 256;
int S_MIN = 96; //96 //0
int S_MAX = 256;

void on_trackbar(int, void*)
{//This function gets called whenever a
 // trackbar position is changed


}

bool laneIsEmpty(Lane lane)
{
	if (lane.numberOfLanes < 2) return true;
	else return false;
}

int process_lines(Mat img, vector<Vec4i> lines, Scalar color = Scalar(255, 0, 0), int thickness = 5)
{
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
extern "C"
JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_SourceClass_processImage(JNIEnv *env, jobject instance, jlong mRgba){
//int processImage(Mat matAddrRgba) {


    Mat &cameraFeed = *(Mat*)mRgba;
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



    int number_of_frames = 0;

    //Mat gray_image = cvtColor
   // while (1) {


    GaussianBlur(cameraFeed, gaussFeed, Size(7, 7), 20); // helps suppress noise

    // get greyscale image

    cvtColor(gaussFeed, greyFeed, CV_BGR2GRAY);


    mask_hls = hlsSelect(gaussFeed, Scalar(H_MIN, L_MIN, S_MIN),
                         Scalar(H_MAX, L_MAX, S_MAX));
    inRange(greyFeed, 240, 255, mask_white); // get white mask
    bitwise_or(mask_white, mask_hls, mask_whls); // get white and yellow mask combined
    bitwise_and(greyFeed, mask_whls, mask_whls_image); // get filtered image


    double low_threshold = 50;
    double high_threshold = low_threshold * 3;
    Canny(mask_whls_image, canny_edges, low_threshold,
          high_threshold); // canny edge detection

    // rho and theta are the distance and angular resolution of the grid in Hough space
    double rho = 1;
    double theta = pi / 180;
    // threshold is minimum number of intersections in a grid for candidate line to go to output
    int thresholdHough = 100;
    double min_line_len = 200;
    double max_line_gap = 200;
    HoughLinesP(canny_edges, hough_lines, rho, theta, thresholdHough, min_line_len,
                max_line_gap);
    Mat line_img(greyFeed.rows, greyFeed.cols, CV_8UC3, Scalar(0, 0, 0));
    //imshow("canny", mask_hls);
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

    static vector<double> cache = {0, 0, 0, 0, 0, 0, 0, 0};
    static bool first_frame = true;
    static int linesCrossed = 0;

    int y_global_min = line_img.rows;
    int y_max = line_img.rows;

    vector<double> l_slope = {};
    vector<double> r_slope = {};
    vector<Vec4i> l_lane = {};
    vector<Vec4i> r_lane = {};

    static vector<Lane> lanes = {};

    double det_slope = 0.5;
    double a = 0.4;

    for (int i = 0; i < hough_lines.size(); i++) {
        //1)
        Vec4i l = hough_lines[i];
        double slope = (double(l[3]) - double(l[1])) /
                       (double(l[2]) - double(l[0])); // (y2-y1)/(x2-x1)
        if (slope == std::numeric_limits<double>::infinity())
            slope = (double(line_img.rows)) / 1;
        double b = 0;

        //slope = std::abs(slope);
        //b = std::abs(b);
        int minIndex = -1;
        if (abs(slope) > det_slope) {

            // MED classifier
            if (slope != std::numeric_limits<double>::infinity()) {
                b = (slope * double(l[0]) - double(l[1])) / slope;
                double x = (slope * b + line_img.rows) / slope;
                double minDist = 50000;
                for (int j = 0; j < lanes.size(); j++) {
                    Lane lane = lanes[j];
                    double lx = lane.getX(line_img.rows);
                    double lB = lane.getB();
                    double dist = abs(
                            pow(x, 2) - 2 * x * lx + pow(lx, 2) + pow(b, 2) - 2 * b * lB +
                            pow(lB, 2));
                    if (dist < minDist) {
                        minDist = dist;
                        minIndex = j;
                    }
                }
            } else {
                b = l[0];
                double minDist = 100;
                for (int j = 0; j < lanes.size(); j++) {
                    Lane lane = lanes[j];
                    double lB = lane.getB();
                    double dist = abs(lB - b);
                    if (dist < minDist) {
                        minDist = dist;
                        minIndex = j;
                    }
                }
            }


            if (minIndex >= 0)
                lanes[minIndex].addLane(slope, b, l[3], l[1], l[0], l[2], line_img.rows,
                                        line_img.cols);
            else {
                Lane lane = Lane(slope, b);
                lanes.push_back(lane);
            }
        }
        line(line_img, Point(int(l[0]), int(l[1])), Point(int(l[2]), int(l[3])),
             Scalar(255, 0, 0), 5); //enable to display all hough lines

    }

    /*if (l_lane.size() == 0 || r_lane.size() == 0)
    {
    return -1; // no lane detected
    }*/


    double y1 = 0;
    double y2 = line_img.rows;

    // compute distance

    // get middle x of the screen
    double xpoint = (double) line_img.cols / 2;

    //Lane* l_lane = NULL;
    //Lane* r_lane = NULL;

    double threshold = 100;

    double distl = threshold * 2;
    double distr = -threshold * 2;

    lanes.erase(std::remove_if(lanes.begin(), lanes.end(), laneIsEmpty), lanes.end());

    for (int i = 0; i < lanes.size(); i++) {
        Lane &lane = lanes[i];
        double x1 = lane.getX(y1, true);
        double x2 = lane.getX(y2, true);

        if (lane.isLaneLine || x2 >= 0 || x2 <= line_img.cols) {
            if (!lane.isFullLine) {
                --lane.buffer;
            } else lane.buffer = 2;
            if (lane.buffer == 0) {
                lane.isLaneLine = false;
            }
        }
        if (lane.isLaneLine) {

            line(line_img, Point(int(x1), int(y1)), Point(int(x2), int(y2)),
                 Scalar(0, 255, 0), 5);

            // get distance
            double dist = xpoint - x2;

            if (lane.lastFrame) lane.lastRight = lane.right;
            if (dist > 0) {
                lane.right = false;
                if (dist < distl) distl = dist;
            } else if (dist < 0) {
                lane.right = true;
                if (dist > distr) distr = dist;
            }
            if (lane.lastFrame) {
                if (lane.lastRight && !lane.right) linesCrossed++;
                else if (!lane.lastRight && lane.right) linesCrossed--;
            }

        }

        lane.oldSlope = lane.getSlope(true);
        lane.oldB = lane.getB(true);
        lane.totalSlope = 0;
        lane.totalB = 0;
        lane.numberOfLanes = 0;
        lane.isFullLine = false;
        lane.lastFrame = true;

    }
    int decision = 0;
    if (linesCrossed < 0) decision = 1;
    else if (linesCrossed > 0) decision = 2;
    if (distl < threshold) decision = 1;
    else if (distr > -threshold) decision = 2;

    //--------------------

   // addWeighted(line_img, 0.8, cameraFeed, 1, 0, result);
   // int font = FONT_HERSHEY_SIMPLEX;
   // string msg;
   // if (decision == 1) msg = "LEFT";
   // else if (decision == 2) msg = "RIGHT";
   // else msg = "ON TRACK";
   // putText(result, msg, Point(10, 500), font, 4, Scalar(255, 255, 255), 2, LINE_AA);

    //imshow("Raw", line_img);
    //imshow("Result", result);
    return decision;


  //  }

   // std::string hello = "Hello +";
}
extern "C"
JNIEXPORT void JNICALL
Java_com_melissarinch_ontrackdemoc_SourceClass_getMat(JNIEnv *env, jobject instance, jlong mRgba, jlong greyfeed){
    Mat &src = *(Mat*)mRgba;
    Mat gaussFeed;
    Mat &greyFeed = *(Mat*)greyfeed;

   if(src.rows > 0 && src.cols > 0) {
       GaussianBlur(src, gaussFeed, Size(7, 7), 20); // helps suppress noise

       cvtColor(gaussFeed, greyFeed, CV_BGR2GRAY);
   }
    std::string hello = "Hello 1";

}

//}