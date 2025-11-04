#include <LedControl.h>
#include <Servo.h>



#define ANALOG_X_PIN A2
#define ANALOG_Y_PIN A3
#define ANALOG_BUTTON_PIN A4

#define ANALOG_X_CORRECTION 128
#define ANALOG_Y_CORRECTION 128

struct analog { 
	 short x, y;
};

analog analog;



// Initiate Micro Servo
Servo myServo;

// Variables
int servoPosition = 90;
const int stepSize = 10;
volatile unsigned long lastJoystickAction = 0;
const int debounce = 250;

const int startTrigPin = 8;  
const int startEchoPin = 7;

const int endTrigPin = 6;
const int endEchoPin = 5;

// measure pulse on the first ultrasonic distance sensor
long startDuration;
int startDistance;

// measure pulse on the second ultrasonic distance sensor
long endDuration;
int endDistance;

// ultrasonic sensor common parameters
int sensorDistance = 5; // centimeters


// Initiate 1055AS 8x8 led matrix
LedControl lc = LedControl(11, 13, 10, 1);

// Define 8x8 matrix numbers
byte digits[10][8] = {
    // number 0
    {B00001111,
    B00001001,
    B00001001,
    B00001001,
    B00001001,
    B00001001,
    B00001001,
    B00001111},

    // number 1
    {B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001},


    // number 2
    {B00001111,
    B00000001,
    B00000001,
    B00000001,
    B00001111,
    B00001000,
    B00001000,
    B00001111},

    // number 3
    {B00001111,
    B00000001,
    B00000001,
    B00000001,
    B00001111,
    B00000001,
    B00000001,
    B00001111},

    // number 4
    {B00001001,
    B00001001,
    B00001001,
    B00001001,
    B00001111,
    B00000001,
    B00000001,
    B00000001},

    // number 5
    {B00001111,
    B00001000,
    B00001000,
    B00001000,
    B00001111,
    B00000001,
    B00000001,
    B00001111},

    // number 6
    {B00001111,
    B00001000,
    B00001000,
    B00001000,
    B00001111,
    B00001001,
    B00001001,
    B00001111},

    // number 7
    {B00001111,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001,
    B00000001},

    // number 8
    {B00001111,
    B00001001,
    B00001001,
    B00001001,
    B00001111,
    B00001001,
    B00001001,
    B00001111},

    // number 9
    {B00001111,
    B00001001,
    B00001001,
    B00001001,
    B00001111,
    B00000001,
    B00000001,
    B00001111}
};


void setup()
{
  lc.shutdown(0, false);   // Wake up MAX7219
  lc.setIntensity(0, 1);   // Brightness (0–15)
  lc.clearDisplay(0);      // Clear display

  myServo.attach(9);
  myServo.write(servoPosition);

  Serial.begin(9600);
  Serial.println("Servo control ready!");

  // pinMode(yellowButton, INPUT_PULLUP);
  // pinMode(redButton, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(yellowButton), turnServoLeft, FALLING);
  // attachInterrupt(digitalPinToInterrupt(redButton), turnServoRight, FALLING);

  // Start pin declaration
  pinMode(startTrigPin, OUTPUT);
	pinMode(startEchoPin, INPUT);

  // End pin declaration
  pinMode(endTrigPin, OUTPUT);
  pinMode(endEchoPin, INPUT);

  // Joystick setup
  // pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);

  
}



void loop()
{
  // // Start sensor
  // digitalWrite(startTrigPin, LOW);
  // delayMicroseconds(5);
  // digitalWrite(startTrigPin, HIGH);
  // delayMicroseconds(5);
  // digitalWrite(startTrigPin, LOW);

  // startDuration = pulseIn(startEchoPin, HIGH);
  // startDistance = startDuration * 0.034 / 2;
  // if (startDistance <= sensorDistance && startDistance >= 1){
  //   Serial.print("Start sensor detected an object: Distance: ");
  //   Serial.print(startDistance);
  //   Serial.println(" cm");
  // }
  

  // // End sensor
  // digitalWrite(endTrigPin, LOW);
  // delayMicroseconds(5);
  // digitalWrite(endTrigPin, HIGH);
  // delayMicroseconds(5);
  // digitalWrite(endTrigPin, LOW);

  // endDuration = pulseIn(endEchoPin, HIGH);
  // endDistance = endDuration * 0.034 / 2;
  // if (endDistance <= sensorDistance && endDistance >= 1){
  //   Serial.print("End sensor detected an object: Distance: ");
  //   Serial.print(endDistance);
  //   Serial.println(" cm");
  // }

  // dont need x axis
  analog.x = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION;
  analog.y = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION;
    
  // analog.button.pressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);

  if (analog.y >= 100){
    turnServoLeft();

    Serial.print("X: ");
    Serial.print(analog.x);
    Serial.print(" ; ");
    Serial.print("Y: ");
    Serial.println(analog.y);
  }

  if (analog.y <= -100){
    turnServoRight();
    
    Serial.print("X: ");
    Serial.print(analog.x);
    Serial.print(" ; ");
    Serial.print("Y: ");
    Serial.println(analog.y);
  }
  delay(200);










  // for (int i = 0; i < 100; i++) {
  //     displayNumber(i);
  //     delay(300);
  // }

}

void displayNumber(int number) {
  int tens = number / 10;
  int ones = number % 10;

  for (int row = 0; row < 8; row++) {
    // Shift the tens digit left by 4 pixels to leave space
    byte left = digits[tens][row] << 4; // shift right for centering
    byte right = digits[ones][row]; // move right digit to right half

    byte combined = left | right;
    lc.setRow(0, row, combined);
  }
}

void turnServoLeft() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastJoystickAction > debounce) {
    if (servoPosition - stepSize >= 0) {
        servoPosition -= stepSize;
        myServo.write(servoPosition);

        Serial.print("Moved left to ");
        Serial.println(servoPosition);
      }
      lastJoystickAction = interruptTime;
  }
}

void turnServoRight() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastJoystickAction > debounce) {
    if (servoPosition + stepSize <= 180) {
        servoPosition += stepSize;
        myServo.write(servoPosition);

        Serial.print("Moved right to ");
        Serial.println(servoPosition);
      }
      lastJoystickAction = interruptTime;
  }
}

byte readAnalogAxisLevel(int pin)
{
	 return map(analogRead(pin), 0, 1023, 0, 255);
} 
	 
bool isAnalogButtonPressed(int pin)
{
	 return digitalRead(pin) == 0;
}