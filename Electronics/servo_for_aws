//upload in arduino board
#include <Servo.h>

Servo myservo; 

const int buttonPin = 2; 
int buttonState = 0;
void setup() {
  // The servo control wire is connected to Arduino D2 pin.
  myservo.attach(9);
  // Servo is stationary.
  myservo.write(0);
  pinMode(buttonPin, INPUT);
  Serial.begin(9600);
}


void loop() {
  // read the state of the pushbutton value:
  buttonState = digitalRead(buttonPin);


  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    Serial.println("Servo Engage");
    myservo.write(0);
    delay(1000);
    myservo.write(50);
    delay(1000);
}
   else {
    // turn LED off:
    Serial.println("Servo DisEngage");
    myservo.write(0);
  }}
