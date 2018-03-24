#include "convert_gray.h"

JNIEXPORT jint JNICALL
Java_com_melissarinch_ontrackdemoc_NativeClass_convertGray(JNIEnv *env, jclass type,
                                                           jlong matAddrRgba, jlong matAddrGray) {


    Mat &mRgb = *(Mat *) matAddrRgba;
    Mat &mGray = *(Mat *) matAddrGray;
    int conv;
    jint retVal;
    conv = toGray(mRgb, mGray);
    retVal = (jint) conv;
    return retVal;

}
int toGray(Mat img, Mat &gray) {
    cvtColor(img, gray, CV_RGB2BGR);
    if (gray.rows == img.rows && gray.cols == img.cols)
        return 1;
}



//
// Created by melis on 3/23/2018.
//


