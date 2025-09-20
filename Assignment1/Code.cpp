// Ultrasonic pins
const int trigPin = 7;
const int echoPin = 6;

// Output pins
const int greenLED = 13;
const int yellowLED = 12;
const int redLED = 8;

// Red LED and Piezo
const int piezoPin = 4;
const int safeBeepInterval = 300;
const int warningBeepInterval = 150;

// Variables
long duration;
int distance;
const int maxDistance = 300;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(piezoPin, OUTPUT);

  // set communication with Serial Monitor
  Serial.begin(9600);
}

void loop() {
  // Trigger ultrasonic pulse
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // Measure echo
  duration = pulseIn(echoPin, HIGH);

  // Convert to cm
  distance = duration * 0.034 / 2;

  // Debug output
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  
  if (distance > maxDistance / 2 && distance <= maxDistance) { 
    // Safe zone logic
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    
    tone(piezoPin, 85);
    delay(safeBeepInterval);
    noTone(piezoPin);
    delay(safeBeepInterval);
    
  } else if (distance > 70 && distance <= maxDistance / 2) {
    // Warning zone logic
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    
    tone(piezoPin, 85);
    delay(warningBeepInterval);
    noTone(piezoPin);
    delay(warningBeepInterval);
  } 
  else if (distance <= 70 && distance > 2) {
    // Danger zone
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
    tone(piezoPin, 85);
  } 
  else {
    // Out of range
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    noTone(piezoPin);
  }
  delay(200);
}
