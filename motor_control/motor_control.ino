#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#define USE_SERIAL Serial
//
const char *ssid_Router     = "*******"; //Enter the router name
const char *password_Router = "*******"; //Enter the router password

String address= "http://165.227.76.232:3000/fag2113/running";
Servo myservo;  // create servo object to control a servo

//Servo motor is going to be on the waist, since the waist doesn't move more than 180 degrees on any point, probably max 45 degrees in either direction.
//Stepper motor is more careful so use it for the wrist of the wushu figure. 


int posVal = 0;    // variable to store the servo position
int servoPin = 15; // Servo motor pin

int outPorts[] = {14, 27, 26, 25}; //Stepper output ports. 

int steps = 0; 
int pos = 0; 
bool servoDir = true;


void setup() {
  USE_SERIAL.begin(115200);

  WiFi.begin(ssid_Router, password_Router);
  USE_SERIAL.println(String("Connecting to ")+ssid_Router);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.println("\nConnected, IP address: ");
  USE_SERIAL.println(WiFi.localIP());
  USE_SERIAL.println("Setup End");
  myservo.setPeriodHertz(50);           // standard 50 hz servo
  myservo.attach(servoPin, 500, 2500);  // attaches the servo on servoPin to the servo object
  for (int i = 0; i < 4; i++) {
    pinMode(outPorts[i], OUTPUT);
  }
}
void loop() {

  if((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(address);
 
    int httpCode = http.GET(); // start connection and send HTTP header
    if (httpCode == HTTP_CODE_OK) { 
        String response = http.getString();
        if (response.equals("false")) {
            // Do not run sculpture, perhaps sleep for a couple seconds
            USE_SERIAL.println("Response was: " + response);
        }
        else if(response.equals("true")) {
            if(pos < 1){
              servoDir = true;
            } else if (pos > 170){
              servoDir = false;
            }
            
            if(servoDir){
              pos++;
            } else{
              pos--;
            }
            moveSteps(servoDir, 6, 3);
            myservo.write(pos);
            USE_SERIAL.println(pos); 
            
            delay(10); 
        }
//        USE_SERIAL.println("Response was: " + response);
    } else {
        USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
//    delay(500); // sleep for half of a second
  }
  
//  for (posVal = 0; posVal <= 180; posVal += 1) { // goes from 0 degrees to 180 degrees
//    // in steps of 1 degree
//    myservo.write(posVal);       // tell servo to go to position in variable 'pos'
//    delay(15);                   // waits 15ms for the servo to reach the position
//  }
//  for (posVal = 180; posVal >= 0; posVal -= 1) { // goes from 180 degrees to 0 degrees
//    myservo.write(posVal);       // tell servo to go to position in variable 'pos'
//    delay(15);                   // waits 15ms for the servo to reach the position
//  }
//  moveSteps(true, 32 * 8, 3);
//  delay(1000);
// 
//  moveSteps(false, 32 * 8, 3);
//  delay(1000);
}



//Suggestion: the motor turns precisely when the ms range is between 3 and 20
void moveSteps(bool dir, int steps, byte ms) {
  for (unsigned long i = 0; i < steps; i++) {
    moveOneStep(dir); // Rotate a step
    delay(constrain(ms,3,20));        // Control the speed
  }
}

void moveOneStep(bool dir) {
  // Define a variable, use four low bit to indicate the state of port
  static byte out = 0x01;
  // Decide the shift direction according to the rotation direction
  if (dir) {  // ring shift left
    out != 0x08 ? out = out << 1 : out = 0x01;
  }
  else {      // ring shift right
    out != 0x01 ? out = out >> 1 : out = 0x08;
  }
  // Output singal to each port
  for (int i = 0; i < 4; i++) {
    digitalWrite(outPorts[i], (out & (0x01 << i)) ? HIGH : LOW);
  }
}

void moveAround(bool dir, int turns, byte ms){
  for(int i=0;i<turns;i++)
    moveSteps(dir,32*64,ms);
}
void moveAngle(bool dir, int angle, byte ms){
  moveSteps(dir,(angle*32*64/360),ms);
}
