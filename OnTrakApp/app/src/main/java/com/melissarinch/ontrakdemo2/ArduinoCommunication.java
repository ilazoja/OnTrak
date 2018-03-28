package com.melissarinch.ontrakdemo2;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnKeyListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

import static android.content.ContentValues.TAG;

public class ArduinoCommunication extends Activity {

    //Declare buttons & editText
    Button functionOne, functionTwo;

    private EditText editText;

    //Memeber Fields
    private BluetoothAdapter btAdapter = null;
    private BluetoothSocket btSocket = null;
    private OutputStream outStream = null;

    // UUID service - This is the type of Bluetooth device that the BT module is
    // It is very likely yours will be the same, if not google UUID for your manufacturer
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // MAC-address of Bluetooth module
    public String newAddress = null;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_arduino_communication);

        addKeyListener();

        //Initialising buttons in the view
        //mDetect = (Button) findViewById(R.id.mDetect);
        functionOne = (Button) findViewById(R.id.functionOne);
        functionTwo = (Button) findViewById(R.id.functionTwo);

        //getting the bluetooth adapter value and calling checkBTstate function
        btAdapter = BluetoothAdapter.getDefaultAdapter();
        checkBTState();

        /**************************************************************************************************************************8
         *  Buttons are set up with onclick listeners so when pressed a method is called
         *  In this case send data is called with a value and a toast is made
         *  to give visual feedback of the selection made
         */

        functionOne.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Function 1 is called");
                sendData("1");
                Toast.makeText(getBaseContext(), "Function 1", Toast.LENGTH_SHORT).show();
            }
        });

        functionTwo.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "Function 2 is called");
                sendData("2");
                Toast.makeText(getBaseContext(), "Function 2", Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "Resuming...");
        // connection methods are best here in case program goes into the background etc

        //Get MAC address from DeviceListActivity
        Intent intent = getIntent();
        newAddress = intent.getStringExtra(DeviceList.EXTRA_DEVICE_ADDRESS);
        Log.d(TAG, "New address");

        // Set up a pointer to the remote device using its address.
        BluetoothDevice device = btAdapter.getRemoteDevice(newAddress);
        Log.d(TAG, "BT device created");

        //Attempt to create a bluetooth socket for comms
        try {
            btSocket = device.createRfcommSocketToServiceRecord(MY_UUID);
            Log.d(TAG, "Creating BT socket...");
        } catch (IOException e1) {
            Toast.makeText(getBaseContext(), "ERROR - Could not create Bluetooth socket", Toast.LENGTH_SHORT).show();
            Log.d(TAG, "Could not create BT socket");
        }

        // Establish the connection.
        try {
            btSocket.connect();
            Log.d(TAG, "Socket connected!");
        } catch (IOException e) {
            try {
                btSocket.close();        //If IO exception occurs attempt to close socket
                Log.d(TAG, "Closing socket - couldn't connect");
            } catch (IOException e2) {
                Toast.makeText(getBaseContext(), "ERROR - Could not close Bluetooth socket", Toast.LENGTH_SHORT).show();
                Log.d(TAG, "Couldn't close socket");
            }
        }

        // Create a data stream so we can talk to the device
        try {
            outStream = btSocket.getOutputStream();
            Log.d(TAG, "Creating output stream...");
        } catch (IOException e) {
            Toast.makeText(getBaseContext(), "ERROR - Could not create bluetooth outstream", Toast.LENGTH_SHORT).show();
            Log.d(TAG, "Could not create o/p stream");
        }
        //When activity is resumed, attempt to send a piece of junk data ('x') so that it will fail if not connected
        // i.e don't wait for a user to press button to recognise connection failure
        sendData("x");
    }

    @Override
    public void onPause() {
        super.onPause();
        Log.d(TAG, "Paused...");
        //Pausing can be the end of an app if the device kills it or the user doesn't open it again
        //close all connections so resources are not wasted

        //Close BT socket to device
        try     {
            btSocket.close();
        } catch (IOException e2) {
            Toast.makeText(getBaseContext(), "ERROR - Failed to close Bluetooth socket", Toast.LENGTH_SHORT).show();
        }
    }
    //takes the UUID and creates a comms socket
    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {

        return  device.createRfcommSocketToServiceRecord(MY_UUID);
    }

    //same as in device list activity
    private void checkBTState() {
        // Check device has Bluetooth and that it is turned on
        if(btAdapter==null) {
            Toast.makeText(getBaseContext(), "ERROR - Device does not support bluetooth", Toast.LENGTH_SHORT).show();
            finish();
        } else {
            if (btAdapter.isEnabled()) {
            } else {
                //Prompt user to turn on Bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }

    // Method to send data
    private void sendData(String message) {
        byte[] msgBuffer = message.getBytes();

        try {
            //attempt to place data on the outstream to the BT device
            outStream.write(msgBuffer);
            Log.d(TAG, "Outstream writing...");
            Log.d(TAG, "Message being sent is " + message);
        } catch (IOException e) {
            //if the sending fails this is most likely because device is no longer there
            Toast.makeText(getBaseContext(), "ERROR - Device not found", Toast.LENGTH_SHORT).show();
            finish();
        }
    }
    public void addKeyListener() {
        Log.d(TAG, "Add key listener method is called");

        // get edittext component
        editText = (EditText) findViewById(R.id.editText1);

        // add a keylistener to keep track user input
        editText.setOnKeyListener(new OnKeyListener() {
            public boolean onKey(View v, int keyCode, KeyEvent event) {

                // if keydown and send is pressed implement the sendData method
                if ((keyCode == KeyEvent.KEYCODE_ENTER) && (event.getAction() == KeyEvent.ACTION_DOWN)) {
                    //I have put the * in automatically so it is no longer needed when entering text
                    sendData('*' + editText.getText().toString());
                    Toast.makeText(getBaseContext(), "Sending text", Toast.LENGTH_SHORT).show();

                    return true;
                }

                return false;
            }
        });
    }

}
