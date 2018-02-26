#include <windows.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

int main()
{
	Mat cameraFeed;

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
					capture.set(CV_CAP_PROP_POS_FRAMES, 0);
					capture.read(cameraFeed);
				}
				number_of_frames = capture.get(CV_CAP_PROP_POS_FRAMES);
			}
		}

		imshow("Raw", cameraFeed);

		waitKey(30);
	}
}