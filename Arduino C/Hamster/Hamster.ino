/* Hamster
   Firmware: 1.3.123
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
#include <math.h>
#include <SerialCommand.h> // Include to parse serial in strings

/* Bluetooth Communications constants */
const byte rxPin = A2;
const byte txPin = A3;

/* Compass Navigations variables */
float currentHeading;
float rotateDegree = 30;
byte val; // For receiving commands from Processing

/* Status LED constants and variables */
const byte statusLEDPins[] = {4, 2, 3}; // Array for Status LED with pins for red, green, and blue
byte status;

/* Drive Train constant and variables */
const byte rightMotor1 = 11;   // PWM control Right Motor -
const byte rightMotor2 = 10;   // PWM control Right Motor +
const byte leftMotor1 = 5;  // PWM control Left Motor +
const byte leftMotor2 = 6;  // PWM control Left Motor -
const int loopSpeed = 50; // 50ms between loops
long loopTimer = 0; // will store last time loop occurred
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
boolean probabilityCheck = false; // Counter for checking if probabilities high threshold
int maxAttempts = 500; // Set max attempts to learn
int learningAttempts = 0; // Set initial attempts to learn
float probability[] = {0.167, 0.167, 0.167, 0.167, 0.167, 0.167}; // Set equal initial probabilities
float probabilityRemainder; // To hold remaining probability when floor has been reached, then split up amoung other probabilities

/* Ultrasonic Rangefinder constants and variables */
const byte ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const byte ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100;
const int safeZone = 40; // 40cm between Hamster and any object
int ultraSensorCM[1]; // will stored distance from object in cm
unsigned int ultraSensorRaw; // will store raw ultrasensor range finder distance

DRV8833 driver = DRV8833(); // Create an instance of the DRV8833:
NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.
SoftwareSerial bluetooth = SoftwareSerial(rxPin, txPin); // RX, TX for bluetooth
SerialCommand SCmd(bluetooth);   // The demo SerialCommand object, using the SoftwareSerial Constructor

Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345); // Create compass object


void setup() {
  /* Initalize pins for bluetooth*/
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  bluetooth.begin(38400);

  /* Setup callbacks for SerialCommand commands */
  SCmd.addCommand("D", drive_command); // Drive command
  SCmd.addCommand("S", set_speed); // Speed command
  SCmd.addCommand("L", learning_max); // Max Learning Attempts command
  SCmd.addCommand("R", learning_reset); // Reset Learning Attempts command
  SCmd.addCommand("T", set_rotateDegree); //Set Rotate Degree command

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

  bluetooth.println(F("\n\rH A M S T E R v1.3.0 <3\n"));
  statusLed(0); // Set status LED to Ready (green)
}

void loop() {
  if (bluetooth.available()>0){
    SCmd.readSerial();     // Process serial commands
  }
  
  /* Measure the time since last loop */
  unsigned long currentLoopMillis = millis(); // record current time
  if (currentLoopMillis - loopTimer > loopSpeed) { // save the last time you looped
    loopTimer += loopSpeed; // update time since last ping

    statusLed(4); // Ping status (light blue)
    ultraSensorRaw = ultraSensor.ping(); // Send ping, get ping time in microseconds (uS).

    /* Hamster checks if ultraSensor sees anything 40 cm in front of it */
    ultraSensorCM[0] = ultraSensorRaw / US_ROUNDTRIP_CM; // Convert ping time to distance in cm (0 = outside set distance range)
    if (ultraSensorCM[0] > 0 && ultraSensorCM[0] <= safeZone) {
      driveTrain(6, dutyCycle, 0, currentHeading); // Stop motors when something is detected within safe zone
      statusLed(1); // Object Avoidance status (Red)

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
            digitalWrite(modeLED, LOW); // Turn off Learning Mode LED
          }
        }

        if (probabilityCheck == false) {
          digitalWrite(modeLED, HIGH); // Turn on Learning Mode LED

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
            }
          }

          learningAttempts++; // Increase learning tracker
          bluetooth.print("C "); bluetooth.println(learningAttempts);
        }
      } else if (modeState == LOW || (learningAttempts >= maxAttempts)) {
        digitalWrite(modeLED, LOW); // Turn off Learning Mode LED
      }

    } else {
      // Go forwards
      driveInstruction = 0; // Drive Forwards
      status = 3; // Wander status (Purple)

      /* Check if we are driving straight */
      if (driveInstruction == 0) { //
        float targetHeading = currentHeading; // Set target heading
        currentHeading = readCompass(); // Find current heading
        float difference = targetHeading - currentHeading;
        if ( difference > -1 && difference < 1) { // Add buffer so this doesn't run all the time
          driveInstruction = adjustHeading(difference); // Determine rotation direction: CCW or CW
        }
      }

      /* Check if we are in Standby Mode */
      modeRoam = digitalRead(roamPin);
      if (modeRoam == HIGH || modeState == HIGH) {
        currentHeading = readCompass(); // Get current heading
        driveTrain(driveInstruction, dutyCycle, rotateDegree, currentHeading);
      }
    }
    statusLed(status);
  }

} // loop() end

void drive_command() {
  int aNumber;
  char *arg;

  arg = SCmd.next();
  if (arg != NULL)  {
    aNumber = atoi(arg);  // Converts a char string to an integer
    if (aNumber >= 0 && aNumber < 7) {
      currentHeading = readCompass(); // Get current heading
      driveTrain(aNumber, dutyCycle, rotateDegree, currentHeading);
    }
  }
}

void set_speed() {
  int aNumber;
  char *arg;

  arg = SCmd.next();
  if (arg != NULL)  {
    aNumber = atoi(arg);  // Converts a char string to an integer
    if (aNumber >= 0 && aNumber <= 100) {
      dutyCycle = aNumber; // Set duty cycle PWM
    }
  }
}

void set_rotateDegree() {
  int aNumber;
  char *arg;

  arg = SCmd.next();
  if (arg != NULL)  {
    aNumber = atoi(arg);  // Converts a char string to an integer
    if (aNumber >= 0 && aNumber <= 359) {
      rotateDegree = aNumber; // Set max attempts to learn
    }
  }
}

void learning_max() {
  int aNumber;
  char *arg;

  arg = SCmd.next();
  if (arg != NULL)  {
    aNumber = atoi(arg);  // Converts a char string to an integer
    if (aNumber >= 0 && aNumber <= 1000) {
      maxAttempts = aNumber; // Set max attempts to learn
    }
  }
}

void learning_reset() {
  learningAttempts = 0; // Reset attempts to learn
  
}

/* Drive Train for 2 motors on opposite sides */
void driveTrain(byte instruction, byte dutyCycle, float rotateDegree, float currentHeading) {
  int motorSpeed = map(dutyCycle, 0, 100, 0, 255); // Map motorSpeed (255 - 0) to dutyCycle (0 - 100)
  float targetHeading;
  float difference;

  switch (instruction) {
    case 0: // Forward
      driver.motorAForward(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;
    case 1: // Backwards
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
      driver.motorAReverse(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;
    case 5: // Rotate left
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

/* Controls status LED */
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

  return headingDegrees;
}

/* Determine which direction we need to rotate: CCW or CW */
byte adjustHeading(float difference) {
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
