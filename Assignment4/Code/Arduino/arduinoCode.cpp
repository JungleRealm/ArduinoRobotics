#include <Servo.h>
#include <Wire.h>
#include <TimerOne.h>
#include <EEPROM.h>

#define ANALOG_X_PIN A4
#define ANALOG_Y_PIN A3
#define ANALOG_BUTTON_PIN A2

const int SERVO_MIN = 100;
const int SERVO_MAX = 160;

struct analog { 
    short x, y;
};
analog analog;

// Micro Servo setup
Servo myServo;
const int servoControl = 9;
int servoPosition = 130;
const int stepSize = 2;

// Buttons
const int autoManualSwitchButtonPin = 2; // Yellow button
const int saveStateButtonPin = 3; // Red button

// Variables
volatile unsigned long lastJoystickAction = 0;
const int debounce = 300;
volatile unsigned long lastYellowInterruptTime = 0;
volatile unsigned long lastRedInterruptTime = 0;
int joystickCenter = 128;
bool autoMode = false;

// EEPROM
const byte MAGIC_NUMBER = 0x42;
const int EEPROM_MAGIC_ADDR = 0;
const int EEPROM_SERVO_ADDR = 1;

// Prevent user input when extracting value from EEPROM
bool startupServoMoving = true;

// Christmas lights setup:
const int CHRISTMAS_BUTTON_PIN = 5;
const int PIEZO_PIN = 13;
const int LED1_R = 12;
const int LED1_B = 11;
const int LED1_G = 10;
const int LED2_R = 8;
const int LED2_B = 7;
const int LED2_G = 6;

int melody[] = {
    659, 659, 659,
    659, 659, 659,
    659, 783, 523, 587, 659
};

int noteDurations[] = {
    200, 200, 400,
    200, 200, 400,
    200, 200, 200, 200, 800
};

bool christmasMode = false;
int noteIndex = 0;
unsigned long lastNoteTime = 0;
bool ledState = false;
const int totalNotes = sizeof(melody) / sizeof(int);

void setup() {
    Serial.begin(9600);

    //myServo.write(servoPosition);
    myServo.attach(servoControl);

    delay(300);
    int savedPos = loadServoPositionFromEEPROM();
    Serial.println("Restoring servo position...");
    for (int pos = servoPosition; pos != savedPos;) {
        if (pos < savedPos) pos++;
        else pos--;
        myServo.write(pos);
        delay(30);
    }
    servoPosition = savedPos;
    Serial.println("Servo restore completed.");
    startupServoMoving = false;

    delay(1000);
    Serial.println("Servo ready!");

    Serial.println("Calibrating joystick...");
    joystickCenter = readAnalogAxisLevel(ANALOG_Y_PIN);
    Serial.print("Joystick Y center = ");
    Serial.println(joystickCenter);

    pinMode(autoManualSwitchButtonPin, INPUT_PULLUP);
    pinMode(saveStateButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(autoManualSwitchButtonPin), yellowButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(saveStateButtonPin), redButton, FALLING);
    pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);
    pinMode(CHRISTMAS_BUTTON_PIN, INPUT_PULLUP);
    pinMode(PIEZO_PIN, OUTPUT);
    pinMode(LED1_R, OUTPUT);
    pinMode(LED1_G, OUTPUT);
    pinMode(LED1_B, OUTPUT);
    pinMode(LED2_R, OUTPUT);
    pinMode(LED2_G, OUTPUT);
    pinMode(LED2_B, OUTPUT);
}

void loop() {
    // Ignore all button presses while EEPROM value is loaded
    if (startupServoMoving) {
        return;
    }

    if (digitalRead(CHRISTMAS_BUTTON_PIN) == LOW && !christmasMode) {
        christmasMode = true;
        noteIndex = 0;
        lastNoteTime = millis();
        delay(200); // debounce
    }

    if (autoMode) {
        int rightLight = analogRead(A0);
        int leftLight = analogRead(A1);
        autoTracking(leftLight, rightLight);
        delay(300);

    } else {
        analog.y = readAnalogAxisLevel(ANALOG_Y_PIN) - joystickCenter;
        if (analog.y >= 120)  turnServoRight();
        if (analog.y <= -120) turnServoLeft();

        if (digitalRead(ANALOG_BUTTON_PIN) == LOW) {
            int target = 130;
            if (servoPosition > target) {
                for (int pos = servoPosition; pos >= target; pos--) {
                    myServo.write(pos);
                    delay(50);
                }
            } else {
                for (int pos = servoPosition; pos <= target; pos++) {
                    myServo.write(pos);
                    delay(50);
                }
            }
        servoPosition = target;
        delay(300);
        }
    }
  if (christmasMode) {
    ChristmasUpdate();
  }
}

void turnServoLeft() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastJoystickAction > debounce) {
        if (servoPosition - stepSize >= SERVO_MIN) {
            servoPosition -= stepSize;
            myServo.write(servoPosition);
        }
        lastJoystickAction = interruptTime;
    }
}

void turnServoRight() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastJoystickAction > debounce) {
        if (servoPosition + stepSize <= SERVO_MAX) {
            servoPosition += stepSize;
            myServo.write(servoPosition);
        }
        lastJoystickAction = interruptTime;
    }
}

void autoLeft() {
    if (servoPosition > SERVO_MIN) {
        servoPosition -= 2;
        myServo.write(servoPosition);
    }
}

void autoRight() {
    if (servoPosition < SERVO_MAX) {
        servoPosition += 2;
        myServo.write(servoPosition);
    }
}

void redButton() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastRedInterruptTime > debounce && !startupServoMoving) {
        saveServoPositionToEEPROM();
        lastRedInterruptTime = interruptTime;
    }
}

void yellowButton() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastYellowInterruptTime > debounce) {
        autoMode = !autoMode;
        if (autoMode) {
            Serial.println("AUTO mode enabled!");
        } else {
            Serial.println("MANUAL mode enabled!");
        }
        lastYellowInterruptTime = interruptTime;
    }
}

void autoTracking(int leftLight, int rightLight) {
    int tolerance = 50;
    int difference = leftLight - rightLight;

    if (abs(difference) < tolerance) {
        return;
    }

    if (difference > 0) {
        autoLeft();
    } else {
        autoRight();
    }
}

void saveServoPositionToEEPROM() {
    EEPROM.write(EEPROM_MAGIC_ADDR, MAGIC_NUMBER);
    EEPROM.write(EEPROM_SERVO_ADDR, servoPosition);
    Serial.print("Saved servo position: ");
    Serial.println(servoPosition);
}

int loadServoPositionFromEEPROM() {
    byte magic = EEPROM.read(EEPROM_MAGIC_ADDR);

    if (magic != MAGIC_NUMBER) {
        Serial.println("EEPROM empty or invalid. Using default position 130.");
        return 130; // dafault servo position value
    }

    int saved = EEPROM.read(EEPROM_SERVO_ADDR);
    // Serial.print("Loaded servo position from EEPROM: ");
    // Serial.println(saved);
    return saved;
}

byte readAnalogAxisLevel(int pin)
{
    return map(analogRead(pin), 0, 1023, 0, 255);
}

void ChristmasUpdate() {
    unsigned long now = millis();
    if (now - lastNoteTime >= noteDurations[noteIndex]) {
        tone(PIEZO_PIN, melody[noteIndex], noteDurations[noteIndex] - 20);

        ledState = !ledState;
        digitalWrite(LED1_R, ledState);
        digitalWrite(LED1_G, !ledState);
        digitalWrite(LED1_B, ledState);

        digitalWrite(LED2_R, !ledState);
        digitalWrite(LED2_G, ledState);
        digitalWrite(LED2_B, !ledState);

        lastNoteTime = now;
        noteIndex++;

        if (noteIndex >= totalNotes) {
            noTone(PIEZO_PIN);
            digitalWrite(LED1_R, LOW);
            digitalWrite(LED1_G, LOW);
            digitalWrite(LED1_B, LOW);
            digitalWrite(LED2_R, LOW);
            digitalWrite(LED2_G, LOW);
            digitalWrite(LED2_B, LOW);
            christmasMode = false;
        }
    }
}