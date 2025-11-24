#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// WiFi setup
const char* ssid = "SSID";
const char* pass = "PASSWORD";
const char* host = "IP_ADDRESS";  
const int port = 5000;

WiFiClient client;

// LM35 pin
const int LM35_PIN = A0;

void setup() {
    Serial.begin(115200);

    // IR sensor I2C
    Wire.begin(D2, D1);
    mlx.begin();

    // WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println("\nConnected!");
}

void loop() {
    // reconnect
    if (!client.connected()) {
        Serial.println("Connecting to server...");
        client.connect(host, port);
        delay(500);
        return;
    }

    // MLX
    float mlxTemp = mlx.readObjectTempC();
    // LM35
    int rawValue = analogRead(LM35_PIN);

    // Convert to voltage and temp
    float voltage = rawValue * (3.3 / 1023.0);
    float lm35Temp = voltage * 100.0;

    // Send data to Java code
    client.print("MLX=");
    client.print(mlxTemp);
    client.print(";LM35=");
    client.print(lm35Temp);
    client.print("\n");
    delay(1000);
}
