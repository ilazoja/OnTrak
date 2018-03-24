package com.melissarinch.ontrackdemoc;

import android.content.Intent;
import android.content.Loader;
import android.graphics.Camera;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.BaseExpandableListAdapter;
import android.widget.TextView;

import com.melissarinch.ontrackdemoc.R;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;


public class MainActivity extends AppCompatActivity{
    private  JavaCameraView javaCameraView;
    Mat mRgba;
    Mat mGray;
  //  BaseLoaderCallback baseLoaderCallback = new BaseLoaderCallback(this) {
//        @Override
//        public void onManagerConnected(int status) {
//
//            switch (status){
//                case BaseLoaderCallback.SUCCESS:
//                    javaCameraView.enableView();
//                    break;
//                default:
//                    super.onManagerConnected(status);
//                    break;
//            }
//        }
//    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
       super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
//        javaCameraView = (JavaCameraView)findViewById(R.id.javaCameraView);
//        javaCameraView.setVisibility(View.VISIBLE);
//        javaCameraView.setCvCameraViewListener(this);
//
//        // Example of a call to a native method
//        //TextView tv = (TextView) findViewById(R.id.sample_text);
//        //tv.setText(stringFromJNI());
        String tring = MainActivity.stringFromJNI();
        Log.i("str", tring);
    }

//    @Override
//    protected void onPause(){
//        super.onPause();
//        if(javaCameraView != null){
//            javaCameraView.disableView();
//        }
//    }
//    protected void onDestroy(){
//        super.onDestroy();
//        if(javaCameraView != null){
//            javaCameraView.disableView();
//        }
//    }
//    protected void onResume(){
//        super.onResume();
//        if(OpenCVLoader.initDebug()){
//            Log.i("success", "OpenCV loaded successfully");
//            baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
//        }
//        else{
//            Log.i("fail", "OpenCV not loaded");
//            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, this, baseLoaderCallback);
//        }
//    }
//
//    /**
//     * A native method that is implemented by the 'native-lib' native library,
//     * which is packaged with this application.
//     */
   public native static String stringFromJNI();




    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java3");

    }
    public void toCamera(View v){
        Intent intent = new Intent(this, CameraActivity.class);
        startActivity(intent);

    }
//
//    public void openCamera(){
//
//    }
//
//    @Override
//    public void onCameraViewStarted(int width, int height) {
//        mRgba = new Mat(height, width, CvType.CV_8UC4);
//    }
//
//    @Override
//    public void onCameraViewStopped() {
//
//    }
//
//    @Override
//    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
//        mRgba = inputFrame.rgba();
//
//        NativeClass.convertGray(mRgba.getNativeObjAddr(), mGray.getNativeObjAddr());
//        NativeClass.ProcessImage();
//        return mGray;
//    }


}
