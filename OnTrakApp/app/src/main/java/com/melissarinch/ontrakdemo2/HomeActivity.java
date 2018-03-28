package com.melissarinch.ontrakdemo2;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;

public class HomeActivity extends AppCompatActivity {

    private static final String TAG = "HomeActivity";


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_home);

    }
    public void toDeviceList(View v){
        Log.d(TAG, "Going from HomeActivity to DeviceList...");
        Intent intent = new Intent(this, DeviceList.class);
        startActivity(intent);
    }
}
