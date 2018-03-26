package com.melissarinch.ontrackdemoc;
import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.MenuItem;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.widget.Toast;

import org.opencv.android.JavaCameraView;
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;


/**
 * Created by melis on 3/23/2018.
 */

public class CameraActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {

        // Used for logging success or failure messages
        private static final String TAG = "OCVSample::Activity";

        // Loads camera view of OpenCV for us to use. This lets us see using OpenCV
        private CameraBridgeViewBase mOpenCvCameraView;

        // Used in Camera selection from menu (when implemented)
        private boolean              mIsJavaCamera = true;
        private MenuItem             mItemSwitchCamera = null;

        // These variables are used (at the moment) to fix camera orientation from 270degree to 0degree
        Mat mRgba;
        Mat mRgbaF;
        Mat mRgbaT;


        private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
            @Override
            public void onManagerConnected(int status) {
                switch (status) {
                    case LoaderCallbackInterface.SUCCESS:
                    {
                        Log.i(TAG, "OpenCV loaded successfully");
                        System.loadLibrary("native-lib");
                        mOpenCvCameraView.enableView();
                    } break;
                    default:
                    {
                        super.onManagerConnected(status);
                    } //break;
                }
            }
        };

        public CameraActivity() {
            Log.i(TAG, "Instantiated new " + this.getClass());
        }



        @Override
        protected void onCreate(Bundle savedInstanceState) {
            Log.i(TAG, "called onCreate");
            super.onCreate(savedInstanceState);
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

            setContentView(R.layout.show_camera);

            // Permissions for Android 6+
            ActivityCompat.requestPermissions(CameraActivity.this,
                    new String[]{Manifest.permission.CAMERA},
                    1);
            mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.show_camera_activity_java_surface_view);

            mOpenCvCameraView.setVisibility(SurfaceView.VISIBLE);
            mOpenCvCameraView.setCvCameraViewListener(this);
           // mOpenCvCameraView.setCvCameraViewListener(this);
            mOpenCvCameraView.setMinimumHeight(400);
            mOpenCvCameraView.setMinimumWidth(400);
            mOpenCvCameraView.setMaxFrameSize(400, 400);
        }

        @Override
        public void onPause()
        {
            super.onPause();
            if (mOpenCvCameraView != null)
                mOpenCvCameraView.disableView();
        }

        @Override
        public void onResume()
        {
            super.onResume();
            if (!OpenCVLoader.initDebug()) {
                Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
                OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_4_0, this, mLoaderCallback);
            } else {
                Log.d(TAG, "OpenCV library found inside package. Using it!");
                mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
            }
        }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 1: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(CameraActivity.this, "Permission denied to read your External storage", Toast.LENGTH_SHORT).show();
                }
                return;
            }
            // other 'case' lines to check for other
            // permissions this app might request
        }
    }


    public void onDestroy() {
            super.onDestroy();
            if (mOpenCvCameraView != null)
                mOpenCvCameraView.disableView();
        }

        public void onCameraViewStarted(int width, int height) {

//            mRgba = new Mat(height, width, CvType.CV_8UC4);
//            mRgbaF = new Mat(height, width, CvType.CV_8UC4);
//            mRgbaT = new Mat(width, width, CvType.CV_8UC4);
        }

        public void onCameraViewStopped() {
          //  mRgba.release();
        }

    // Used to load the 'native-lib' library on application startup.
    static {
        //System.loadLibrary("native-lib");
       // System.loadLibrary("opencv_java3");

    }
        public Mat onCameraFrame(CvCameraViewFrame inputFrame) {

            // TODO Auto-generated method stub
            mRgba = inputFrame.rgba();
            // Rotate mRgba 90 degrees
          //  Core.transpose(mRgba, mRgbaT);
            //Imgproc.resize(mRgbaT, mRgbaF, mRgbaF.size(), 0,0, 0);
            //Core.flip(mRgbaF, mRgba, 1 );

//          String a =  SourceClass.processImage(mRgba.getNativeObjAddr());

            Mat greyFeed = new Mat(mRgba.height(),mRgba.width(), CvType.CV_8UC4);
            int b = mRgba.rows();
            int c = mRgba.cols();
            Log.i("rows", ((Integer) b).toString());
            Log.i("cols", ((Integer) c).toString());
            int dec = 9;
            if( mRgba.rows() > 0 && mRgba.cols()>0) {
               dec = SourceClass.processImage(mRgba.getNativeObjAddr());
            }

            Log.i("Cam", ((Integer) dec).toString());
           // greyFeed.release();


          //  Log.i("camera",a);


                return mRgba;



             // This function must return
        }


    }

