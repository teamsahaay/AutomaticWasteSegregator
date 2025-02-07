#include "BMP.h"
#include "OV7670.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP32Servo.h>

// WiFi credentials
const char* ssid = "Hotspot";
const char* password = "qwerty";

// Define ports for the servers
const int cameraPort = 8080;
const int controlPort = 8081;

// Camera pins
const int SIOD = 21; // SDA
const int SIOC = 22; // SCL
const int VSYNC = 19;
const int HREF = 23;
const int XCLK = 25;
const int PCLK = 26;
const int D0 = 34;
const int D1 = 35;
const int D2 = 32;
const int D3 = 18;
const int D4 = 33;
const int D5 = 12;
const int D6 = 27;
const int D7 = 13;
int obj = 0;
// Servo and stepper
const int SerPin = 14;
const int stepPin = 4; // Example stepper step pin
const int dirPin = 15;  // Example stepper direction pin

int pos = 0;
int enable=0;
int flag = 1;
int currentAngle = 0; 
int temp_angle = 0 ;
int angle = 0;
float stepPerAngle = 0.05625; //1/800 steps
int   numstep;

// Global server instances
WiFiServer cameraServer(cameraPort);
WiFiServer controlServer(controlPort);

unsigned char bmpHeader[BMP::headerSize];

// Camera instance
OV7670 *camera;

void serveCamera() {
  WiFiClient client = cameraServer.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(
              "<style>body{margin: 0}\nimg{height: 100%; width: auto}</style>"
              "<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
              "<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /camera")) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:image/bmp");
          client.println();

          client.write(bmpHeader, BMP::headerSize);
          client.write(camera->frame, camera->xres * camera->yres * 2);
        }
      }
    }
    client.stop();
  }
}

void handleControlRequest(WiFiClient client) {
  String request = client.readStringUntil('\r');
  client.flush();

  // Extract the command
  int object_number = request.substring(request.indexOf('=') + 1).toInt();
  Serial.println("Received control data: " + String(object_number));
  obj = object_number;
 

  // Respond to the control request
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/plain");
  client.println();
  client.println("Control command received");
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize the servo
  
  pinMode(SerPin, OUTPUT); // Initialize the servo position
  digitalWrite(dirPin,HIGH);
  digitalWrite(SerPin,LOW);
  // Initialize stepper pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Initialize the camera
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

  // Start servers
  cameraServer.begin();
  controlServer.begin();
}

void loop() {
  camera->oneFrame();
  serveCamera();

  // Handle control server requests
  WiFiClient controlClient = controlServer.available();
  if (controlClient) {
    handleControlRequest(controlClient);
    controlClient.stop();
    Serial.println(obj);
    switch(obj) { 
        case 1:
            angle = 0;
            enable=1;
            Serial.println("0");
           break;

        case 2:
           angle = 60;
           Serial.println("60");
           break;

        case 3:
            angle = 120;
            Serial.println("120");
           // handle ‘3’
           break;
        case 4 :
            angle = 180;
            Serial.println("180");
           // handle ‘3’
           break;
        case 5 :
            angle = -60;
            Serial.println("240");
           // handle ‘3’
           break;
         case 6 :
            angle = -120;
            Serial.println("300");
           // handle ‘3’
           break;
   }
  }
   int n= 0;
   int numstep =0 ;
    if( currentAngle != angle || enable== 1 ){


        if( currentAngle < angle){

            digitalWrite(dirPin,HIGH);

             n = angle - currentAngle;

            numstep = n / stepPerAngle;

        }

        else if( currentAngle > angle){

            digitalWrite(dirPin,LOW);

            n = currentAngle - angle;

            if( angle == 0){

               n =currentAngle;

            }

            numstep = n / stepPerAngle;

        }



        for(int x = 0; x < numstep; x++) {

           digitalWrite(stepPin,HIGH);

           delayMicroseconds(1000);

           digitalWrite(stepPin,LOW);

           delayMicroseconds(1000);

        }
        currentAngle = angle;
        Serial.println("Rotated");
        Serial.println(angle);
        Serial.println(temp_angle);
        
        if (flag == 1){
          digitalWrite(SerPin, HIGH);
          delay(1000);
          digitalWrite(SerPin, LOW);
          flag = 0 ;
        }
        else {
          digitalWrite(SerPin, LOW);
          flag = 1 ; 
        }
        enable=0;
    }
    temp_angle = angle;
    angle = 0; 
    delay(2000); // Delay after stepper rotation
  delay(100); // Optional delay to avoid excessive polling
}
