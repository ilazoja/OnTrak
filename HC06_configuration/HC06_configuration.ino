#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 2); // RX, TX - DI goes to RX and D0 goes to TX.
int LED_right_1 = 8;
int LED_left_0 = 9;
int LED_right_3 = 10;
int LED_left_2 = 11;

int input = 0;

String command = ""; // Stores response of the HC-06 Bluetooth device


void setup() {
  // Open serial communications:
  Serial.begin(9600);
  Serial.println("Type AT commands!");
  
  // The HC-06 defaults to 9600 according to the datasheet.
  mySerial.begin(9600);
  pinMode(LED_right_1, OUTPUT);
  pinMode(LED_left_0, OUTPUT); 
    pinMode(LED_right_3, OUTPUT);
  pinMode(LED_left_2, OUTPUT); 
}

void loop() {
  // Read device output if available.
  if (mySerial.available()) {
    while(mySerial.available()) { // While there is more to be read, keep reading.
      command += (char)mySerial.read();
      input = command.toInt();
      

      if (input == 0 || input == 1 || input == 2 || input == 3)
      {
         Serial.println(input);
         if (input == 1)
         {
            digitalWrite(LED_right_1, HIGH);
            delay(5000);
            digitalWrite(LED_right_1, LOW);
         }
         else if (input == 0)
         {
            digitalWrite(LED_left_0, HIGH);
            delay(5000);
            digitalWrite(LED_left_0, LOW);
         }
         else if (input == 3)
         {
          digitalWrite(LED_right_1, HIGH);
          digitalWrite(LED_right_3, HIGH);
          delay(5000);
          digitalWrite(LED_right_1, LOW);
          digitalWrite(LED_right_3, LOW);
         }
         else if (input == 2)
         {
          digitalWrite(LED_left_0, HIGH);
          digitalWrite(LED_left_2, HIGH);
          delay(5000);
          digitalWrite(LED_left_0, LOW);
          digitalWrite(LED_left_2, LOW);
         }
      }
      else
      {
        Serial.println("Bruh. 1s or 0s");
      }
    }
    
    Serial.println(command);
    command = ""; // No repeats
  }
  
  // Read user input if available.
  if (Serial.available()){
    delay(10); // The delay is necessary to get this working!
    mySerial.write(Serial.read());
  }


}
