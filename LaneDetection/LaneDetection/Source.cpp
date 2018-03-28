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
int H_MIN = 26;
int H_MAX = 100;
int L_MIN = 0; //0 //121
int L_MAX = 265;
int S_MIN = 0; //96 //0
int S_MAX = 265;

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

    //GaussianBlur(img, gaussFeed, Size(23, 23), 100); // helps suppress noise

    // get greyscale image

    cvtColor(img, greyFeed, CV_BGR2GRAY);

    mask_hls = hlsSelect(img, Scalar(H_MIN, L_MIN, S_MIN), Scalar(H_MAX, L_MAX, S_MAX));
    //inRange(greyFeed, 200, 255, mask_white); // get white mask
    //bitwise_or(mask_white, mask_hls mask_whls); // get white and yellow mask combined
    bitwise_and(greyFeed, mask_hls, mask_whls_image); // get filtered image

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


    //imshow("canny", mask_whls);
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

    mask_hls = hlsSelect(gaussFeed, Scalar(0, 117, 0), Scalar(60, 256, 256));
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
    //imshow("obstacle", canny_edges);
    return white_pixels.size() > threshold;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_SourceClass_processImage(JNIEnv *env, jobject instance, jlong mRgba, jlong line1, jlong mask_hl){
//int processImage(Mat matAddrRgba) {


    Mat &cameraFeed = *(Mat*)mRgba;
    bool detectObjects = false;

    Mat result;

    static bool started = false;
    static int linesCrossed = 0;

    int searchRange = 200;
    static bool obstacleDetectedLastFrame = false;
    static bool obstacleDetectedCurrentFrame = false;
    Mat &line_img = *(Mat*)line1;

    int decision = 0;

    static vector<Lane> lanes = {};

    if (detectObjects)
    {
        obstacleDetectedLastFrame = obstacleDetectedCurrentFrame;
        obstacleDetectedCurrentFrame = find_obstacles(cameraFeed(Range(0, line_img.rows / 2), Range(line_img.cols / 4, line_img.cols * 3 / 4)));
        if (obstacleDetectedCurrentFrame && !obstacleDetectedLastFrame) linesCrossed--;
        if (!obstacleDetectedCurrentFrame && obstacleDetectedLastFrame) linesCrossed++;
    }

    if (!started || lanes.size() == 0 || true)
    {
        Mat sub_img = cameraFeed(Range(cameraFeed.rows/2, cameraFeed.rows), Range(0, cameraFeed.cols));
        update_line(sub_img, cameraFeed.rows/2, 0, line_img, lanes);
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

    return decision;
    //--------------------

    //addWeighted(line_img, 0.8, cameraFeed, 1, 0, result);
   // int font = FONT_HERSHEY_SIMPLEX;
   // string msg;
   // if (decision == 1) msg = "LEFT";
   // else if (decision == 2) msg = "RIGHT";
   // else msg = "ON TRACK";
   // putText(result, msg, Point(10, 500), font, 4, Scalar(255, 255, 255), 2, LINE_AA);

    //imshow("Raw", line_img);
    //imshow("Result", result);

    //Mat gray_image = cvtColor
   // while (1) {
    // get greyscale image



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