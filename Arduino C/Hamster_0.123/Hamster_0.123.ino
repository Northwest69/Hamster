/* Hamster 0.1 created by Peter Chau
   June 5, 2015
   
   Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder
   
   Hamster checks if ultraSensor sees anything 30 cm in front of it, and quickly rotates right if it does. Otherwise, Hamster drives forward slowly.
   It's scared of things!
    
   To do:
   Make Hamster look around then determine the best direction to drive
   Install compass
   Make hamster drive staight
   Add On/Off switch and function
   Add On/Off LED indicator
   Implement a neuro network for obsticale avoidance

   Log: 

   7/6/2015
   Installed DVR8833 library
   Mapped dutyCycle to motorSpeed
   I seem to have fired my Arduino UNO. Computer won't recognize it however when plugged into the wall via USB, the board works fine.

   7/7/2015
   Created Breadboard Diagram
   Created Schematic

   7/8/2015
   Arduino Uno now functioning - Window's driver needed to be installed. Gave it a fresh battery charge aswell.
   Updated Breadboard and schematic - was missing Uno VIN.

   7/9/2015
   Create DC Motor drive function. void driveTrain(int instruction, int dutyCycle, int period)
   Implemented driveTrain()
   Created better serial messages
   Reworked ultrasonic rangefinder pinging as a state function
   ultrasonic rangefind needs to be calibrated. wtf. it's slow.
   ok now the robot works but is really dumb lol. it runs into stuff... lag time... momentum... go and :speed up ping

   7/10/2015
   Ultrasonic rangefinder working well, however there is lag between when it detects an object and when it takes action.
      Calibrated Sensor
   Poor wheel alignment
   Implemented a status LED
 */

#include <NewPing.h> // Include NewPing to handle ultraSensor data
#include <DRV8833.h> // Include DRV8833 Dual Motor Driver Carrier - Pololu  

/* Ultrasonic Rangefinder constants and variables */
const int ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const int ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100;
const int pingSpeed = 50; // 50ms between pings
const int safeZone = 40; // 30cm between Hamster and any object
long pingTimer = 0; // will store last time ping occurred
int ultraSensorCM; // will stored distance from object in cm

/* Drive Train constant and variables */
const int rightMotor1 = 11;   //PWM control Right Motor -
const int rightMotor2 = 10;   //PWM control Right Motor +
const int leftMotor1 = 5;  //PWM control Left Motor +
const int leftMotor2 = 6;  //PWM control Left Motor -
int driveInstruction = 0; //Initial Command
int dutyCycle = 0; // 0% duty cycle PWM
int motorSpeed;

/* Status LED constants and variables */
const int statusLED[] = {4,2,1}; // Array for Status LED with pins for red, green, and blue
int status = 0;

DRV8833 driver = DRV8833(); // Create an instance of the DRV8833:
NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.

void setup(){
  Serial.begin(9600);             // Start Serial connection
/* Attach the motors to the input pins: */
  driver.attachMotorA(rightMotor1, rightMotor2);
  driver.attachMotorB(leftMotor1, leftMotor2);
/* Attach status LED to output pins */
  for(int x = 0; x <3; x++) {
  pinMode(statusLED[x], OUTPUT);
  }
  
  Serial.println("H A M S T E R V0.123\n");
  statusLed(0); // Set status LED to Ready (green)
}

void loop(){

  unsigned long currentMillis = millis(); //record current time

/* Measure the distanace to closest object */
  if(currentMillis - pingTimer > pingSpeed) { // save the last time you pinged
  pingTimer += pingSpeed; // update time since last ping
  statusLed(2); // Object avoidance status (light red)
  unsigned int ultraSensorRaw = ultraSensor.ping(); // Send ping, get ping time in microseconds (uS).
  ultraSensorCM = ultraSensorRaw / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)
  }
/* Hamster checks if ultraSensor sees anything 20 cm in front of it, and rotates right if it does. Otherwise, Hamster drives forward.*/
  if(ultraSensorCM > 0 && ultraSensorCM <= safeZone){     
/* Print to Serial ultraSensorCM */
    Serial.print("Distance to Closest Object: ");
    Serial.print(ultraSensorCM); 
    Serial.println("cm");
    driveInstruction = 3; // rotate right
    dutyCycle = 75;
    status = 1; // Object avoidance status (red)
  } else {
    driveInstruction = 1; // go forwards
    dutyCycle = 50;
    status = 0; // Ready status (green)
  }
  
  driveTrain(driveInstruction, dutyCycle);
  statusLed(status);
  Serial.println("--------------------------------------------\n");

} // loop() end

/* Drive Train for 2 motors on opposite sides */
void driveTrain(int instruction, int dutyCycle){
  motorSpeed = map(dutyCycle, 0, 100, 0, 255); // Map motorSpeed (255 - 0) to dutyCycle (0 - 100)
  
/* Drive Train data */
  Serial.print("Duty Cycle: ");
  Serial.print(dutyCycle); 
  Serial.print("%\n");

  switch(instruction){
   case 0: // Stop
    Serial.println("Stop\n");
    driver.motorAStop();
    driver.motorBStop();
    break;
   case 1: // Forward
    Serial.println("Forward\n");
    driver.motorAForward(motorSpeed);
    driver.motorBForward(motorSpeed);
    break;
   case 2: // Backwards
    Serial.println("Backward\n");
    driver.motorAReverse(motorSpeed);
    driver.motorBReverse(motorSpeed);
    break; 
   case 3: // Rotate right
    Serial.println("Rotate Right\n");
    driver.motorAReverse(motorSpeed);
    driver.motorBForward(motorSpeed);
    break;
   case 4: // Rotate left
    Serial.println("Rotate Left\n");
    driver.motorAForward(motorSpeed);
    driver.motorBReverse(motorSpeed);
    break;     
  }
  return;
}

/* Controls status LED*/
void statusLed(int status){
  switch(status){
  case 0: // set to LED green to indicate status
  digitalWrite(statusLED[1], LOW);
  digitalWrite(statusLED[0], LOW); 
  if(digitalRead(statusLED[2]) != HIGH) {
    digitalWrite(statusLED[2], HIGH);
  }
  break;  
  case 1: // set to LED red to indicate status
  digitalWrite(statusLED[2], LOW);
  digitalWrite(statusLED[1], LOW); 
  if(digitalRead(statusLED[0]) != HIGH) {
    digitalWrite(statusLED[0], HIGH);
  }
  break;

  case 2: // set to LED blue to indicate status
  digitalWrite(statusLED[0], LOW);
  digitalWrite(statusLED[2], LOW); 
  if(digitalRead(statusLED[1]) != HIGH) {
    digitalWrite(statusLED[1], HIGH);
  }
  break;

  case 3: // set to LED purple to indicate status
  digitalWrite(statusLED[1], LOW); 
  if(digitalRead(statusLED[2]) != HIGH) {
    digitalWrite(statusLED[2], HIGH);
  }
  if(digitalRead(statusLED[0]) != HIGH) {
    digitalWrite(statusLED[0], HIGH);
  }
  break; 
  case 4: // set to LED light blue to indicate status
  digitalWrite(statusLED[0], LOW); 
  if(digitalRead(statusLED[2]) != HIGH) {
    digitalWrite(statusLED[2], HIGH);
  }
  if(digitalRead(statusLED[1]) != HIGH) {
    digitalWrite(statusLED[1], HIGH);
  }
  break; 
  case 5: // set to LED light red to indicate status
  digitalWrite(statusLED[2], LOW); 
  if(digitalRead(statusLED[0]) != HIGH) {
    digitalWrite(statusLED[0], HIGH);
  }
  if(digitalRead(statusLED[1]) != HIGH) {
    digitalWrite(statusLED[1], HIGH);
  }
  break; 
  }
}
