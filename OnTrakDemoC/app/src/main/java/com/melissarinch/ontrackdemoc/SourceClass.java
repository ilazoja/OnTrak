package com.melissarinch.ontrackdemoc;

import org.opencv.core.Mat;

/**
 * Created by melis on 3/23/2018.
 */

public class SourceClass {

    public native static int processImage(long mRgba);
    public native static void getMat(long mRgba, long greyFeed);


    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");

    }
}
