#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <EEPROM.h>

// Initialize the GY-906 infrared sensor communication through the I2C bus
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

// Pins
// Celcius and Fahrenheit toggle (yellow button)
const int buttonUnitPin = 2;
// Increase calibration offset (red button)
const int buttonOffsetPin = 3;

// Variables
volatile bool showFahrenheit = false;
volatile unsigned long lastInterruptTime = 0;
volatile bool updateFlag = false;
float calibrationOffset = 0.0;

// EEPROM Addresses
// Celcius (C) or Fahrenheit (F) mode byte in memory[0]
const int EEPROM_ADDR_MODE = 0;
// 4 bytes reserved in EEPROM memory to store a float of the temperature offset in memory[4, 5, 6, 7]
const int EEPROM_ADDR_OFFSET = 4;
// Magic number is stored in memory[100]
const int EEPROM_MAGIC_ADDR = 100;
// A most basic validation byte
const byte EEPROM_MAGIC_VALUE = 0x42;

// Startup configurations
void setup() {
    Serial.begin(9600);
    delay(2000);
    Wire.begin();
    if (!mlx.begin()) {
        Serial.println("Error: MLX90614 not detected! Check wiring.");
        // stop execution until sensor connected
        while (1); 
    }
    delay(1000);
    lcd.begin(16, 2);
    lcd.print("Temp Monitor");
    delay(1000);
    lcd.clear();

    // Load EEPROM values
    if (EEPROM.read(EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE) {
        byte savedMode = EEPROM.read(EEPROM_ADDR_MODE);
        EEPROM.get(EEPROM_ADDR_OFFSET, calibrationOffset);
        showFahrenheit = (savedMode == 1);
    } else {
        showFahrenheit = false;
        calibrationOffset = 0.0;
        EEPROM.write(EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
        EEPROM.update(EEPROM_ADDR_MODE, 0);
        EEPROM.put(EEPROM_ADDR_OFFSET, calibrationOffset);
    }

    // Button setup
    pinMode(buttonUnitPin, INPUT_PULLUP);
    pinMode(buttonOffsetPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttonUnitPin), toggleUnit, FALLING);
    attachInterrupt(digitalPinToInterrupt(buttonOffsetPin), increaseOffset, FALLING);

    // Setup timer interrupts
    // Timer interrupt happens every 2 seconds
    Timer1.initialize(2000000);
    Timer1.attachInterrupt(onTimer);
}

// Main execution loop
void loop() {
    if (updateFlag) {
        displayTemperature();
        updateFlag = false;
    }
}

// Timer interrupt function
void onTimer() {
    updateFlag = true;
}

/*
This function does the following:
    1. Reads the temperature and adds a calibration offset to it;
    2. Prints the temperature on the LCD;
    3. Shows calibration value on the LCD;
    4. Get the timestamp since start of code execution;
    5. Send all the data over Serial port;
*/
void displayTemperature() {
    // Step 1 - read the temperature
    float objectTemp = mlx.readObjectTempC() + calibrationOffset;

    // Step 2 - display the temperature on the LCD
    lcd.setCursor(0, 0);
    lcd.print("Obj: ");
    if (showFahrenheit) {
        lcd.print(objectTemp * 9.0 / 5.0 + 32.0, 1);
        lcd.print(" F   ");
    } else {
        lcd.print(objectTemp, 1);
        lcd.print(" C   ");
    }

    // Step 3 - display the calibration offset on the LCD
    lcd.setCursor(0, 1);
    lcd.print("Calib: ");
    lcd.print(calibrationOffset, 1);
    lcd.print(" C ");

    // Step 4 - measure the timestamp
    unsigned long ms = millis();
    int hours = (ms / 3600000) % 24;
    int minutes = (ms / 60000) % 60;
    int seconds = (ms / 1000) % 60;

    // Step 5 - Send all data through serial port
    Serial.print((hours < 10 ? "0" : ""));
    Serial.print(hours);
    Serial.print(":");
    Serial.print((minutes < 10 ? "0" : ""));
    Serial.print(minutes);
    Serial.print(":");
    Serial.print((seconds < 10 ? "0" : ""));
    Serial.print(seconds);
    Serial.print(" Temp: ");
    Serial.print(objectTemp, 1);
    Serial.print(" C | Mode: ");
    Serial.print(showFahrenheit ? "Fahrenheit" : "Celcius");
    Serial.print(" | Offset: ");
    Serial.print(calibrationOffset, 1);
    Serial.println(" C");

}

// Change displayed units
void toggleUnit() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 1000) {
        showFahrenheit = !showFahrenheit;
        lastInterruptTime = interruptTime;
        EEPROM.update(EEPROM_ADDR_MODE, showFahrenheit ? 1 : 0);
    }
}

// Increase offset value
void increaseOffset() {
    unsigned long interruptTime = millis();
    if (interruptTime - lastInterruptTime > 1000) {
        calibrationOffset += 0.2;
        if (calibrationOffset > 5.0) {
            calibrationOffset = 0.0; // wrap around
        }
        EEPROM.put(EEPROM_ADDR_OFFSET, calibrationOffset);
        lastInterruptTime = interruptTime;
    }
}
