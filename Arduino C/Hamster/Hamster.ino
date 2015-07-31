/* Hamster 0.3.2 created by Peter Chau
   Start Date: June 5, 2015
   Project: Alpha

   Hamster checks if ultraSensor sees anything 40 cm in front of it. If it does, it chooses an random action based on a set of probabilities. Otherwise, Hamster drives forward.

   When Hamster is in 'Learning Mode', it evaluates it's actions and modifies the probability set until it's tried 100 times. The blue light is on when it's in 'Learning Mode'!

   Data is sent to PC via bluetooth serial terminal.

   Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder, Bluetooth Shield HC-06, and HMC5883L Triple axis compass
*/
#include <SoftwareSerial.h> // Include Software Serial to allow programming and bluetooth at the same time
#include <Bounce2.h> // Include Bounce 2 to handle button bounce
#include <NewPing.h> // Include NewPing to handle ultraSensor data
#include <DRV8833.h> // Include DRV8833 Dual Motor Driver Carrier - Pololu  
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>

/* Ultrasonic Rangefinder constants and variables */
const int ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const int ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100;
const int pingSpeed = 50; // 50ms between pings
const int safeZone = 40; // 40cm between Hamster and any object
long pingTimer = 0; // will store last time ping occurred
unsigned int ultraSensorRaw; // will store raw ultrasensor range finder distance
int ultraSensorCM[1]; // will stored distance from object in cm

/* Drive Train constant and variables */
const int rightMotor1 = 11;   // PWM control Right Motor -
const int rightMotor2 = 10;   // PWM control Right Motor +
const int leftMotor1 = 5;  // PWM control Left Motor +
const int leftMotor2 = 6;  // PWM control Left Motor -
int driveInstruction = 6; // Stop as initial command
int dutyCycle = 0; // Set initial 0% duty cycle PWM
int motorSpeed;

/* Status LED constants and variables */
const int statusLED[] = {4, 2, 3}; // Array for Status LED with pins for red, green, and blue
int status;

/* Neural Network constants and variables */
const int modeButton = 8; // Momentary switch for putting Hamster in learn mode
const int modeLED = 9; // LED to indicate what mode we are in
int modeState = LOW; // Set inital state to drive mode (learn mode off)
int lastModeState = LOW; // Previous mode
float probability[] = {0.250, 0.250, 0.250, 0.250}; // Set equal initial probabilities
float probabilityThreshold = 0.75; // Set max probability threshold to 75%
float probabilityRemainder; // To hold remaining probability when floor has been reached, then split up amoung other probabilities
float probabilityError = 0.01; // Set probability error to 1%
int probabilityCheck = 0; // Counter for checking if probabilities high threshold
int maxAttempts = 500; // Set max attempts to learn
int learningAttempts = 0; // Set initial attempts to learn

/* Bluetooth Communications */
int rxPin = A2;
int txPin = A3;

/* Compass Navigations */
float currentHeading;
float rotateDegree;
float targetDegree;

DRV8833 driver = DRV8833(); // Create an instance of the DRV8833:
NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.
Bounce bouncer = Bounce(); // Create a bounce object
SoftwareSerial bluetooth(rxPin, txPin); // RX, TX for bluetooth
Adafruit_HMC5883_Unified compass = Adafruit_HMC5883_Unified(12345); // Create compass object

void setup() {
  /* Initalize pins for bluetooth*/
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  bluetooth.begin(9600);             // Start Serial connection
  pinMode(modeButton, INPUT); // Initialize mode button and LED
  bouncer .attach(modeButton);
  bouncer .interval(250);
  pinMode(modeLED, OUTPUT);

  /* Initialise the sensor */
  if (!compass.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while (1);
  }

  /* Attach the motors to the input pins: */
  driver.attachMotorA(rightMotor1, rightMotor2);
  driver.attachMotorB(leftMotor1, leftMotor2);

  /* Attach status LED to output pins */
  for (int x = 0; x < 3; x++) {
    pinMode(statusLED[x], OUTPUT);
  }

  bluetooth.println("H A M S T E R v0.3.2 <3\n");
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

  currentHeading = readCompass(); // Get current heading

  /* Hamster checks if ultraSensor sees anything 40 cm in front of it, and rotates right if it does. Otherwise, Hamster drives forward.*/
  ultraSensorCM[0] = ultraSensorRaw / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)
  if (ultraSensorCM[0] > 0 && ultraSensorCM[0] <= safeZone) {
    statusLed(1); // Object Avoidance status (Red)
    /* Print to Serial ultraSensorCM */
    bluetooth.print("Distance to Closest Object: ");
    bluetooth.print(ultraSensorCM[0]);
    bluetooth.println("cm");

    driveTrain(6, dutyCycle, 0, currentHeading); // Stop motors when something is in our safe zone

    /* Feedforward Neural Network with Back Proprogation */
    driveInstruction = weightedRandom(probability); // Use probability to pick an action and perform it
    dutyCycle = 75; // zip zip
    /* Determine if we will need to rotate */
    if (driveInstruction == 2) {
      rotateDegree = 30;
    } else if (driveInstruction == 3) {
      rotateDegree = 30;
    } else {
      rotateDegree = 0;
    }
    driveTrain(driveInstruction, dutyCycle, rotateDegree, currentHeading);

    /* Set to Learning Mode if mode button is pressed*/
    bouncer .update(); // Updates mode button status
    modeState = bouncer .read();

    if (modeState == HIGH) {

      /* Check if any probability has reached 75% */
      for (int x = 0; x < 4; x++) {
        if (probability[x] >= probabilityThreshold - probabilityError) {
          probabilityCheck++;
          bluetooth.println("Probability Threshold Reached");
          bluetooth.print("Probability: "); // Print current drive train probabilities
          for (int x = 0; x < 4; x++) {
            bluetooth.print(probability[x]);
            bluetooth.print(" ");
          }
          bluetooth.println("(Stop, Forward, Backwards, Rotate Right, Rotate Left)");
        }
      }
      if (probabilityCheck == 0) {

        /* Use Neural Network if Hamster tried to learn less than max attempts limit */
        if ( learningAttempts < maxAttempts) {
          digitalWrite(modeLED, HIGH); // Turn on Learning Mode LED
          bluetooth.println("Learning Mode");
          bluetooth.print("Learning Attempts: "); // Print current attempts
          bluetooth.println(learningAttempts);

          ultraSensorRaw = ultraSensor.ping(); // Ping and save it to ultraSensorCM[1]
          ultraSensorCM[1] = ultraSensorRaw / US_ROUNDTRIP_CM;

          if (ultraSensorCM[1] == ultraSensorCM[0]) { // Check if action was successful based on change in distance
            for (int x = 0; x < 4; x++) { // Decrease drive instruction's probability
              if (x == driveInstruction) {
                probability[x] = probability[x] - 0.01;
              } else {
                probability[x] = probability[x] + 0.0025;
              }
              status = 5; // Action Failed status (light red)
            }
          }
          else if (ultraSensorCM[1] < ultraSensorCM[0]) {
            for (int x = 0; x < 4; x++) { // Decrease drive instruction's probability
              if (x == driveInstruction) {
                probability[x] = probability[x] - 0.01;
              } else {
                probability[x] = probability[x] + 0.0025;
              }
              status = 5; // Action Failed status (light red)
            }
          }
          else {
            for (int x = 0; x < 4; x++) { // Increase drive instruction's probability
              if (x == driveInstruction) {
                probability[x] = probability[x] + 0.01;
              } else {
                probability[x] = probability[x] - 0.0025;
              }
              status = 2; // Action Success status (Blue)
            }
          }

          /* Check if any probabilityFloor has been reached*/
          for (int x = 0; x < 4; x++) {
            if (probability[x] <= probabilityError) {
              /* Split remaining probability up between other probabilities */
              probabilityRemainder = probability[x];
              probability[x] = 0;
              for (int x = 0; x < 4; x++) {
                if (probability != 0) {
                  probability[x] += probabilityRemainder / 5;
                }
              }
              bluetooth.print("Probability[");
              bluetooth.print(x + 1);
              bluetooth.println("] has reached Probability Floor");
            }
          }

          bluetooth.print("Probability: "); // Print current drive train probabilities
          for (int x = 0; x < 4; x++) {
            bluetooth.print(probability[x]);
            bluetooth.print(" ");
          }
          bluetooth.println("(Stop, Forward, Backwards, Rotate Right, Rotate Left)");

          learningAttempts++; // Increase learning tracker
        }
      }
    }
  } else {
    /* Check if we are driving straight */
    if (driveInstruction == 0) { //
      float targetHeading = currentHeading; // Set target heading
      bluetooth.print("Target Heading: "); bluetooth.println(targetHeading);
      currentHeading = readCompass(); // Find current heading
      float difference = targetHeading - currentHeading;
      if ( difference > -1 && difference < 1) { // Add buffer so this doesn't run all the time
        driveInstruction = adjustHeading(difference); // Determine rotation direction: CCW or CW
      }
    } else {

      // Go forwards at 85% duty cycle
      driveInstruction = 0; // Drive Forwards
      dutyCycle = 85;
      status = 3; // Wander status (Purple)
    }
    driveTrain(driveInstruction, dutyCycle, 0, currentHeading);
  }
  statusLed(status);
  bluetooth.println("\n~~~~~~~~~~\n");

  /* Should learning mode be switched off? */
  if (modeState == LOW || learningAttempts >= maxAttempts) {
    digitalWrite(modeLED, LOW); // Turn off Learning Mode LED
  }
} // loop() end

/* Drive Train for 2 motors on opposite sides */
void driveTrain(int instruction, int dutyCycle, float rotateDegree, float currentHeading) {
  motorSpeed = map(dutyCycle, 0, 100, 0, 255); // Map motorSpeed (255 - 0) to dutyCycle (0 - 100)

  /* Drive Train data */
  //  Serial.print("Duty Cycle: ");
  //  Serial.print(dutyCycle);
  //  Serial.print("%\t");
float targetHeading;
float difference;
int rotateDirection;

  switch (instruction) {

    case 0: // Forward
      bluetooth.println("Forward");
      driver.motorAForward(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;

    case 1: // Backwards
      bluetooth.println("Backward");
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
        bluetooth.print("Target Heading: "); bluetooth.println(targetHeading);
          /* CW */
          bluetooth.print("Rotate Right");
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
        bluetooth.print("Target Heading: "); bluetooth.println(targetHeading);
          /* CCW */
          bluetooth.println("Rotate Left");
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
      bluetooth.println("Rotate Right");
      driver.motorAReverse(motorSpeed);
      driver.motorBForward(motorSpeed);
      break;

    case 5: // Rotate left
      bluetooth.println("Rotate Left");
      driver.motorAForward(motorSpeed);
      driver.motorBReverse(motorSpeed);
      break;

    case 6: // Stop
      bluetooth.println("Stop");
      driver.motorAStop();
      driver.motorBStop();
      break;
  }
  return;
}

/* Controls status LED*/
void statusLed(int status) {
  switch (status) {
    case 0: // set to LED green to indicate status
      digitalWrite(statusLED[1], LOW);
      digitalWrite(statusLED[0], LOW);
      if (digitalRead(statusLED[2]) != HIGH) {
        digitalWrite(statusLED[2], HIGH);
      }
      break;
    case 1: // set to LED red to indicate status
      digitalWrite(statusLED[2], LOW);
      digitalWrite(statusLED[1], LOW);
      if (digitalRead(statusLED[0]) != HIGH) {
        digitalWrite(statusLED[0], HIGH);
      }
      break;

    case 2: // set to LED blue to indicate status
      digitalWrite(statusLED[0], LOW);
      digitalWrite(statusLED[2], LOW);
      if (digitalRead(statusLED[1]) != HIGH) {
        digitalWrite(statusLED[1], HIGH);
      }
      break;

    case 3: // set to LED purple to indicate status
      digitalWrite(statusLED[1], LOW);
      if (digitalRead(statusLED[2]) != HIGH) {
        digitalWrite(statusLED[2], HIGH);
      }
      if (digitalRead(statusLED[0]) != HIGH) {
        digitalWrite(statusLED[0], HIGH);
      }
      break;
    case 4: // set to LED light blue to indicate status
      digitalWrite(statusLED[0], LOW);
      if (digitalRead(statusLED[2]) != HIGH) {
        digitalWrite(statusLED[2], HIGH);
      }
      if (digitalRead(statusLED[1]) != HIGH) {
        digitalWrite(statusLED[1], HIGH);
      }
      break;
    case 5: // set to LED light red to indicate status
      digitalWrite(statusLED[2], LOW);
      if (digitalRead(statusLED[0]) != HIGH) {
        digitalWrite(statusLED[0], HIGH);
      }
      if (digitalRead(statusLED[1]) != HIGH) {
        digitalWrite(statusLED[1], HIGH);
      }
      break;
  }
}

/* Weighted Random Choice function  */
int weightedRandom(float* weights) {
  float seed = random(3.14); // seed the countdown with pi (yum)
  float choice = seed * 1; // reduce the seed and save into countdown
  for (int x = 0; x < 4; x++) { // minus each probability from choice
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
  
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");
  
  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
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

  bluetooth.print("Heading (degrees): "); bluetooth.println(headingDegrees);

  return headingDegrees;
}

/* Determine which direction we need to rotate: CCW or CW */
int adjustHeading(float difference) {
  bluetooth.print("Difference: "); bluetooth.print(difference);
  if (difference < -270) {
    bluetooth.println(" CW");
    return 4;
  } else if (difference > -270 && difference < 0) {
    bluetooth.println(" CCW");
    return 5;
  } else if (difference < 270 && difference > 0) {
    bluetooth.println(" CW");
    return 4;
  } else if (difference > 270 && difference > 0) {
    bluetooth.println(" CCW");
    return 5;
  }
}



