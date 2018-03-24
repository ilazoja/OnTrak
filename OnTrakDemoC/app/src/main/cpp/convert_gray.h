//
// Created by melis on 3/23/2018.
//

#include <jni.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;
#ifndef ONTRAKDEMOC_CONVERT_GRAY_H
#define ONTRAKDEMOC_CONVERT_GRAY_H
extern "C"{
#endif //ONTRAKDEMOC_CONVERT_GRAY_H
int toGray(Mat img, Mat &gray);
JNIEXPORT jint JNICALL Java_com_melissarinch_ontrackdemoc_NativeClass_convertGray(JNIEnv *env, jclass type,
        jlong matAddrRgba, jlong matAddrGray);

}
