package com.melissarinch.ontrackdemoc;

import org.opencv.core.Mat;

/**
 * Created by melis on 3/21/2018.
 */

public class NativeClass {

    public native static int Lane(Mat matAddrRgba);

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");

    }


}
