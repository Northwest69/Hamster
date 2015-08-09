/* Hamster
   Firmware: 1.0.0
   Created by Peter Chau
   Start Date: June 5, 2015
   Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder, Bluetooth Shield HC-06, and HMC5883L Triple axis compass
*/

#include <avr/pgmspace.h> // Include to use PROGMEM and Flash memory
#include <SoftwareSerial.h> // Include Software Serial to allow programming and bluetooth at the same time
#include <NewPing.h> // Include NewPing to handle ultraSensor data
#include <DRV8833.h> // Include DRV8833 Dual Motor Driver Carrier - Pololu  
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

/* Bluetooth Communications constants */
const byte rxPin = A2;
const byte txPin = A3;

/* Compass Navigations variables */
float currentHeading;
float rotateDegree = 30;

/* Status LED constants and variables */
const byte statusLEDPins[] = {4, 2, 3}; // Array for Status LED with pins for red, green, and blue
byte status;

/* Drive Train constant and variables */
const byte rightMotor1 = 11;   // PWM control Right Motor -
const byte rightMotor2 = 10;   // PWM control Right Motor +
const byte leftMotor1 = 5;  // PWM control Left Motor +
const byte leftMotor2 = 6;  // PWM control Left Motor -
byte driveInstruction;
byte dutyCycle = 75; // Set initial 0% duty cycle PWM

/* Learning and Roam Mode constants and variables */
const byte modeSwitch = 8; // Momentary switch for putting Hamster in learn mode
const byte modeLED = 9; // LED to indicate what mode we are in
const byte roamPin = 7; // Momentary switch for putting Hamster in learn mode
boolean modeRoam = LOW; // Set inital state to drive mode (learn mode off)
boolean modeState = LOW; // Set inital state to drive mode (learn mode off)

/* Neural Network constants and variables */
const float probabilityThreshold = 0.75; // Set max probability threshold to 75%
const float probabilityFloor = 0.01; // Set probability error to 1%
const int maxAttempts = 500; // Set max attempts to learn
boolean probabilityCheck = false; // Counter for checking if probabilities high threshold
int learningAttempts = 0; // Set initial attempts to learn
float probability[] = {0.167, 0.167, 0.167, 0.167, 0.167, 0.167}; // Set equal initial probabilities
float probabilityRemainder; // To hold remaining probability when floor has been reached, then split up amoung other probabilities

/* Ultrasonic Rangefinder constants and variables */
const byte ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const byte ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100;
const int pingSpeed = 50; // 50ms between pings
const int safeZone = 40; // 40cm between Hamster and any object
int ultraSensorCM[1]; // will stored distance from object in cm
long pingTimer = 0; // will store last time ping occurred
unsigned int ultraSensorRaw; // will store raw ultrasensor range finder distance

DRV8833 driver = DRV8833(); // Create an instance of the DRV8833:
NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.
SoftwareSerial bluetooth(rxPin, txPin); // RX, TX for bluetooth
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345); // Create compass object

/* Save strings to Flash Memory */
const char heading[] PROGMEM = "Heading: ";
const char difference[] PROGMEM = "Difference: ";
const char targetHeading[] PROGMEM = "\r\nTarget Heading: ";
const char forward[] PROGMEM = "Forward";
const char backward[] PROGMEM = "Backward";
const char rotateRightBy[] PROGMEM = "Rotate Right by ";
const char rotateLeftBy[] PROGMEM = "Rotate Left by ";
const char rotateRight[] PROGMEM = "Rotate Right";
const char rotateLeft[] PROGMEM = "Rotate Left";
const char distance[] PROGMEM = "Distance to Closest Object: ";
const char cm[] PROGMEM = " cm";
const char thresholdReached[] PROGMEM = "Probability Threshold Reached";
const char learningMode[] PROGMEM = "Learning Mode";
const char learningA[] PROGMEM = "Learning Attempts: ";
const char probability1[] PROGMEM = "Probability[";
const char probabilityF[] PROGMEM = "] has reached the Probability Floor";
const char probabilities[] PROGMEM = "Probability: ";
const char percent[] PROGMEM = "% ";
const char probabilityS1[] PROGMEM = "\n\rForward, Backwards, Rotate Right by ";
const char probabilityS2[] PROGMEM = ", Rotate Left by ";
const char probabilityS3[] PROGMEM = ", Rotate Right, Rotate Left, Stop";
const char newnew[] PROGMEM = "\r\n";
const char* const string_table[] PROGMEM = {heading, difference, targetHeading, forward, backward, rotateRightBy, rotateLeftBy, rotateRight, rotateLeft, distance, cm, thresholdReached, learningMode, learningA, probability1, probabilityF, probabilities, percent, probabilityS1, probabilityS2, probabilityS3, newnew}; // Then set up a table to refer to your strings.
char buffer[30];    // make sure this is large enough for the largest string it must hold

void setup() {
  /* Initalize pins for bluetooth*/
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  bluetooth.begin(9600);

  /* Initialize mode button and LED */
  pinMode(modeSwitch, INPUT);
  pinMode(modeLED, OUTPUT);

  /* Initialise the sensor */
  if (!compass.begin()) {
    while (1);
  }

  /* Attach the motors to the input pins: */
  driver.attachMotorA(rightMotor1, rightMotor2);
  driver.attachMotorB(leftMotor1, leftMotor2);

  /* Attach status LED to output pins */
  for (byte x = 0; x < 3; x++) {
    pinMode(statusLEDPins[x], OUTPUT);
  }

  bluetooth.println(F("\n\rH A M S T E R v1.0.1 <3\n"));
  statusLed(0); // Set status LED to Ready (green)
}

void loop() {

  /* Measure the distanace to closest object */
  unsigned long currentMillis = millis(); //record current time
  if (currentMillis - pingTimer > pingSpeed) { // save the last time you pinged
    pingTimer += pingSpeed; // update time since last ping
    statusLed(4); // Ping status (light blue)
    ultraSensorRaw = ultraSensor.ping(); // Send ping, get ping time in microseconds (uS).
  }

  /* Hamster checks if ultraSensor sees anything 40 cm in front of it */
  ultraSensorCM[0] = ultraSensorRaw / US_ROUNDTRIP_CM; // Convert ping time to distance in cm (0 = outside set distance range)
  if (ultraSensorCM[0] > 0 && ultraSensorCM[0] <= safeZone) {
    driveTrain(6, dutyCycle, 0, currentHeading); // Stop motors when something is detected within safe zone
    statusLed(1); // Object Avoidance status (Red)
    printString(9); // Print to Serial ultraSensorCM
    bluetooth.print(ultraSensorCM[0]);
    printString(10);

    /* Use probability to pick an object avoidance action and perform it */
    driveInstruction = weightedRandom(probability);
    driveTrain(driveInstruction, dutyCycle, rotateDegree, currentHeading);

    modeState = digitalRead(modeSwitch); // What mode are we in? Learning or Drive mode.

    /* Use Forewardfeed Back Proprogation Neural Network if Hamster is in Learning Mode and has tried to learn less than max learning attempts */
    if (modeState == HIGH && (learningAttempts < maxAttempts)) {

      /* Check if any probability has reached the probability threshold */
      for (byte x = 0; x < 6; x++) {
        if (probability[x] >= probabilityThreshold) {
          probabilityCheck = true;
          printString(11);
          digitalWrite(modeLED, LOW); // Turn off Learning Mode LED
        }
      }

      if (probabilityCheck == false) {
        digitalWrite(modeLED, HIGH); // Turn on Learning Mode LED
        printString(12);
        printString(13); // Print current attempts
        bluetooth.println(learningAttempts);

        ultraSensorRaw = ultraSensor.ping(); // Ping and save it to ultraSensorCM[1]
        ultraSensorCM[1] = ultraSensorRaw / US_ROUNDTRIP_CM;

        if (ultraSensorCM[1] == ultraSensorCM[0]) { // Check if action was successful based on change in distance
          for (byte x = 0; x < 6; x++) { // Decrease drive instruction's probability
            if (x == driveInstruction) {
              probability[x] = probability[x] - 0.01;
            } else {
              probability[x] = probability[x] + 0.00167;
            }
            status = 5; // Action Failed status (light red)
          }
        }
        else if (ultraSensorCM[1] < ultraSensorCM[0]) {
          for (byte x = 0; x < 6; x++) { // Decrease drive instruction's probability
            if (x == driveInstruction) {
              probability[x] = probability[x] - 0.01;
            } else {
              probability[x] = probability[x] + 0.00167;
            }
            status = 5; // Action Failed status (light red)
          }
        }
        else {
          for (byte x = 0; x < 6; x++) { // Increase drive instruction's probability
            if (x == driveInstruction) {
              probability[x] = probability[x] + 0.01;
            } else {
              probability[x] = probability[x] - 0.00167;
            }
            status = 2; // Action Success status (Blue)
          }
        }

        /* Check if any probabilityFloor has been reached*/
        for (byte x = 0; x < 6; x++) {
          if (probability[x] <= probabilityFloor) {
            /* Split remaining probability up between other probabilities */
            probabilityRemainder = probability[x];
            probability[x] = 0;
            for (byte x = 0; x < 6; x++) {
              if (probability != 0) {
                probability[x] += probabilityRemainder / 6;
              }
            }
            printString(14); bluetooth.print(x); printString(15);
          }
        }

        learningAttempts++; // Increase learning tracker
      }
    } else if (modeState == LOW || (learningAttempts >= maxAttempts)) {
      digitalWrite(modeLED, LOW); // Turn off Learning Mode LED
    }

    /* Print current drive train probabilities */
    printString(16);
    for (byte x = 0; x < 6; x++) {
      bluetooth.print(probability[x] * 100);
      printString(17);
    }
    printString(18); bluetooth.print(rotateDegree); printString(19); bluetooth.print(rotateDegree); printString(20);

  } else {
    // Go forwards
    driveInstruction = 0; // Drive Forwards
    status = 3; // Wander status (Purple)

    /* Check if we are in Standby Mode */
    modeRoam = digitalRead(roamPin);
    if (modeRoam == HIGH || modeState == HIGH) {
      /* Check if we are driving straight */
      if (driveInstruction == 0) { //
        float targetHeading = currentHeading; // Set target heading
        currentHeading = readCompass(); // Find current heading
        float difference = targetHeading - currentHeading;
        if ( difference > -1 && difference < 1) { // Add buffer so this doesn't run all the time
          driveInstruction = adjustHeading(difference); // Determine rotation direction: CCW or CW
        }
      }

      currentHeading = readCompass(); // Get current heading
      driveTrain(driveInstruction, dutyCycle, rotateDegree, currentHeading);
    }
  }
  statusLed(status);
  printString(21);
} // loop() end

/* Drive Train for 2 motors on opposite sides */
void driveTrain(byte instruction, byte dutyCycle, float rotateDegree, float currentHeading) {
  int motorSpeed = map(dutyCycle, 0, 100, 0, 255); // Map motorSpeed (255 - 0) to dutyCycle (0 - 100)
  float targetHeading;
  float difference;

  switch (instruction) {
    case 0: // Forward
      printString(3);
      driver.motorAForward(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;
    case 1: // Backwards
      printString(4);
      driver.motorAReverse(motorSpeed);
      driver.motorBReverse(motorSpeed);
      break;
    case 2: // Rotate CW
      if ((currentHeading + rotateDegree) >= 360) {
        targetHeading = currentHeading + rotateDegree - 360; // Set target heading
      } else {
        targetHeading = currentHeading + rotateDegree; // Set target heading
      }
      difference = targetHeading - currentHeading;
      while ((difference < -15) || (difference > 15)) { // Add buffer so this doesn't run all the time
        printString(2); bluetooth.println(targetHeading);
        /* CW */
              printString(5); bluetooth.println(rotateDegree);
        driver.motorAReverse(motorSpeed);
        driver.motorBForward(motorSpeed);
        currentHeading = readCompass(); // Find current heading
        difference = targetHeading - currentHeading;
        if ((difference < 10) && (difference > -10)) {
          break;
        }
      }
      break;
    case 3: // Rotate CCW
      if ((currentHeading - rotateDegree) < 0) {
        targetHeading = currentHeading - rotateDegree + 360; // Set target heading
      } else {
        targetHeading = currentHeading - rotateDegree; // Set target heading
      }
      difference = targetHeading - currentHeading;
      while ( (difference < -15) || (difference > 15)) { // Add buffer so this doesn't run all the time
        printString(2); bluetooth.println(targetHeading);
        /* CCW */
        printString(6); bluetooth.println(rotateDegree);
        driver.motorAForward(motorSpeed);
        driver.motorBReverse(motorSpeed);

        currentHeading = readCompass(); // Find current heading
        difference = targetHeading - currentHeading;
        if ((difference < 10) && (difference > -10)) {
          break;
        }
      }
      break;
    case 4: // Rotate right
      printString(7);
      driver.motorAReverse(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;
    case 5: // Rotate left
      printString(8);
      driver.motorAForward(motorSpeed);
      driver.motorBReverse(motorSpeed);
      break;
    case 6: // Stop
      driver.motorAStop();
      driver.motorBStop();
      break;
  }
  return;
}

/* Controls status LED*/
void statusLed(byte status) {
  switch (status) {
    case 0: // set to LED green to indicate status
      digitalWrite(statusLEDPins[1], LOW);
      digitalWrite(statusLEDPins[0], LOW);
      if (digitalRead(statusLEDPins[2]) != HIGH) {
        digitalWrite(statusLEDPins[2], HIGH);
      }
      break;
    case 1: // set to LED red to indicate status
      digitalWrite(statusLEDPins[2], LOW);
      digitalWrite(statusLEDPins[1], LOW);
      if (digitalRead(statusLEDPins[0]) != HIGH) {
        digitalWrite(statusLEDPins[0], HIGH);
      }
      break;
    case 2: // set to LED blue to indicate status
      digitalWrite(statusLEDPins[0], LOW);
      digitalWrite(statusLEDPins[2], LOW);
      if (digitalRead(statusLEDPins[1]) != HIGH) {
        digitalWrite(statusLEDPins[1], HIGH);
      }
      break;
    case 3: // set to LED purple to indicate status
      digitalWrite(statusLEDPins[1], LOW);
      if (digitalRead(statusLEDPins[2]) != HIGH) {
        digitalWrite(statusLEDPins[2], HIGH);
      }
      if (digitalRead(statusLEDPins[0]) != HIGH) {
        digitalWrite(statusLEDPins[0], HIGH);
      }
      break;
    case 4: // set to LED light blue to indicate status
      digitalWrite(statusLEDPins[0], LOW);
      if (digitalRead(statusLEDPins[2]) != HIGH) {
        digitalWrite(statusLEDPins[2], HIGH);
      }
      if (digitalRead(statusLEDPins[1]) != HIGH) {
        digitalWrite(statusLEDPins[1], HIGH);
      }
      break;
    case 5: // set to LED light red to indicate status
      digitalWrite(statusLEDPins[2], LOW);
      if (digitalRead(statusLEDPins[0]) != HIGH) {
        digitalWrite(statusLEDPins[0], HIGH);
      }
      if (digitalRead(statusLEDPins[1]) != HIGH) {
        digitalWrite(statusLEDPins[1], HIGH);
      }
      break;
  }
}

/* Weighted Random Choice  */
byte weightedRandom(float* weights) {
  float seed = random(3.14); // seed the countdown with pi (yum)
  float choice = seed * 1; // reduce the seed and save into countdown
  for (byte x = 0; x < 6; x++) { // minus each probability from choice
    choice -= weights[x];
    if (choice < 0) {
      return x; // return weight index
    }
  }
}

/* Read Triple axis compass */
float readCompass() {
  /* Get a new sensor event */
  sensors_event_t event;
  compass.getEvent(&event);

  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  float declinationAngle = 0.22;
  heading += declinationAngle;

  // Correct for when signs are reversed.
  if (heading < 0)
    heading += 2 * PI;
  // Check for wrap due to addition of declination.
  if (heading > 2 * PI)
    heading -= 2 * PI;

  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180 / M_PI;
  
    printString(0); bluetooth.println(headingDegrees);
  return headingDegrees;
}

void printString(char string){
    strcpy_P(buffer, (char*)pgm_read_word(&(string_table[string])));
    if (string == 3 || string == 4 || string == 7 || string == 8 || string == 10 || string == 11 || string == 12 || string == 15 || string == 20){
    bluetooth.println(buffer);
      } else {
    bluetooth.print(buffer);
      }
    }

/* Determine which direction we need to rotate: CCW or CW */
byte adjustHeading(float difference) {
  printString(1); bluetooth.println(difference);
  if (difference < -270) {
    return 4;
  } else if (difference > -270 && difference < 0) {
    return 5;
  } else if (difference < 270 && difference > 0) {
    return 4;
  } else if (difference > 270 && difference > 0) {
    return 5;
  }
}
