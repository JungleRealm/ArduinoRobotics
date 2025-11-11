#include <LedControl.h>
#include <Servo.h>

// define joystick pins
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
const int servoControl = 9;

// Variables
int servoPosition = 90;
const int stepSize = 10;
volatile unsigned long lastJoystickAction = 0;
const int debounce = 300;

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
bool movingDetectedFlag = false;
const int sensorDistance = 5; // centimeters
const unsigned long cooldown = 800; //ms
unsigned long cooldownTimer = 0;

// variables to measure time when signal was detected
unsigned long startSignalFoundTime;
unsigned long endSignalFoundTime;

// Distance between sensors
int distanceBetweenSensors = 10; // ~10 cm

// Piezo setup
const int piezoPin = 3;

// Led setup
const int redLed = 12;
const int blueLed = 4;

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

void setup(){

  // Wake up MAX7219 8x8 matrix led display 
  lc.shutdown(0, false);
  // Set display brightness
  lc.setIntensity(0, 1);
  // Clear display on start
  lc.clearDisplay(0);

  // Attach servo control to pin9
  myServo.attach(servoControl);
  myServo.write(servoPosition);

  Serial.begin(9600);
  Serial.println("Servo ready!");

  // Start pin declaration
  pinMode(startTrigPin, OUTPUT);
	pinMode(startEchoPin, INPUT);

  // End pin declaration
  pinMode(endTrigPin, OUTPUT);
  pinMode(endEchoPin, INPUT);

  // Joystick setup
  // pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);

  // Speed limit sound and lights
  pinMode(piezoPin, OUTPUT);
  pinMode(redLed, OUTPUT);
  pinMode(blueLed, OUTPUT);
}

void loop()
{
  // Start sensor
  digitalWrite(startTrigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(startTrigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(startTrigPin, LOW);

  startDuration = pulseIn(startEchoPin, HIGH);
  startDistance = startDuration * 0.034 / 2;
  if (startDistance <= sensorDistance && startDistance >= 1 && movingDetectedFlag == false){
    startSignalFoundTime = millis();
    movingDetectedFlag = true;
  }

  // End sensor
  digitalWrite(endTrigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(endTrigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(endTrigPin, LOW);

  endDuration = pulseIn(endEchoPin, HIGH);
  endDistance = endDuration * 0.034 / 2;
  if (endDistance <= sensorDistance && endDistance >= 1 && movingDetectedFlag == true){
    endSignalFoundTime = millis();
    calculateSpeed(endSignalFoundTime - startSignalFoundTime);
    movingDetectedFlag = false;
  }

  // dont need x axis and button
  //analog.x = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION;
  analog.y = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION;
  // analog.button.pressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);

  if (analog.y >= 100){
    turnServoLeft();
  }

  if (analog.y <= -100){
    turnServoRight();
  }
}

void calculateSpeed(unsigned long deltaTime){
  // cm/ms
  float speedCmMs = distanceBetweenSensors / (float)deltaTime;
  // m/s
  float speedMs = (distanceBetweenSensors / 100.0) / (deltaTime / 1000.0);
  // km/h
  float speedKmh = speedMs * 3.6;

  if (speedKmh >= 8){
    Serial.print("Delta time = ");
    Serial.print(deltaTime);
    Serial.print(" ms | Speed: ");
    Serial.print(speedCmMs, 5);
    Serial.print(" cm/ms | ");
    Serial.print(speedMs, 3);
    Serial.print(" m/s | ");
    Serial.print(speedKmh, 2);
    Serial.println(" km/h");

    displayNumber((int) speedKmh);
    policeSiren();
  } else {
    displayNumber((int) speedKmh);
  }
  
}

void policeSiren() {
  for (int i = 0; i < 3; i++) {
    // rising pitch + red LED
    for (int freq = 400; freq <= 1000; freq += 20) {
      tone(piezoPin, freq);
      digitalWrite(redLed, HIGH);
      digitalWrite(blueLed, LOW);
      delay(10);
    }

    // falling pitch + blue LED
    for (int freq = 1000; freq >= 400; freq -= 20) {
      tone(piezoPin, freq);
      digitalWrite(redLed, LOW);
      digitalWrite(blueLed, HIGH);
      delay(10);
    }
  }
  noTone(piezoPin);
  digitalWrite(redLed, LOW);
  digitalWrite(blueLed, LOW);
}

void displayNumber(int number) {
  int tens = number / 10;
  int ones = number % 10;

  for (int row = 0; row < 8; row++) {
    // Shift the tens digit left by 4 pixels to leave space
    byte left = digits[tens][row] << 4;
    byte right = digits[ones][row];
    byte combined = left | right;
    lc.setRow(0, row, combined);
  }
}

void turnServoLeft() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastJoystickAction > debounce) {
    if (servoPosition - stepSize >= 45) {
        servoPosition -= stepSize;
        myServo.write(servoPosition);
      }
      lastJoystickAction = interruptTime;
  }
}

void turnServoRight() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastJoystickAction > debounce) {
    if (servoPosition + stepSize <= 135) {
        servoPosition += stepSize;
        myServo.write(servoPosition);
      }
      lastJoystickAction = interruptTime;
  }
}

byte readAnalogAxisLevel(int pin)
{
	 return map(analogRead(pin), 0, 1023, 0, 255);
}

// Maybe add later 
// bool isAnalogButtonPressed(int pin)
// {
// 	 return digitalRead(pin) == 0;
// }