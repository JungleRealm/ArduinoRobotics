# Project idea
Project idea - create a small prototype of a distance-based parking assistant.

## Problem
When I am parking my car, I cant physically see the distance left between my car and another vehicle or object behind me. A device that would inform me of that distance would surely make parking easier.

## Used components
1x Arduino Uno;
1x Breadboard;
1x Ultrasonic distance sensor (4-pin);
1x Piezo/Buzzer;
3x LEDs;
3x 220 ohm resistors;
Wires;

## Wiring photo
Witing photo is attached to the project. It can be located in the same directory as the end project by the name "Wiring_photo.png".

## Current functionality
When the simulation is started in tinkercad environment, ultrasonic distance sensor starts measuring distance up to 300 centimeters in front of it in a cone. By moving the object in the cone, we are able to get different behavior:
1. When the object is further than 150 centinements away, you will notice only the green LED turned on and the piezo buzzer has long delays between sounds.
2. When the object is closer than 150 centimeters away, but further than 70 centimenets - the green LED will turn off and a yellow LED will light up. The buzzer will also buzz more frequently indicating that the object got closer.
3. When the object is closer than 70 centimeters - the yellow LED will turn off, the red LED will light on. The buzzer will continuesly buzz informing the person that the object is very close to the sensor.
4. If no object is present in the cone, all the lights will be off and the buzzer will not make any sounds.

## Future improvements
For now everything works as expected. However, moving forward I would like to integrate the following functionalities:
1. A more reliable way to tell the distance between the sensor and the object and display it to the user in text form;
2. A more pleasant buzzer;
3. An additional ultrasonic sensor in the front of the breadboard so I can have both sensors working at the same time (or even 4 of them to cover all sides);
4. When both sensors are working, I would need a way to tell which of the sensors is sending the signals so I can tell front to back apart;
5. If we eventually start building physical projects, adding a motor and wheels would be interesting as well;

## Used articles and resources
1. A tutorial about buzzers and how to use them - https://sensorkit.arduino.cc/sensorkit/module/lessons/lesson/04-the-buzzer
2. A tutorial about ultrasonix sensors and how to use them - https://projecthub.arduino.cc/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-7cabe1
3. A place to look for project ideas - https://projecthub.arduino.cc/
4. A beginner tutorial about Arduino - https://www.youtube.com/watch?v=JnJIKX5J0Cc
