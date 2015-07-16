/* Hamster 0.1 created by Peter Chau
   June 5, 2015
   
   Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder
   
   Hamster checks if ultraSensor sees anything 30 cm in front of it, and quickly rotates right if it does. Otherwise, Hamster drives forward slowly.
   It's scared of things!
    
   To do:
   
   Make Hamster look around then determine the best direction to drive
   Create DC Motor drive function
   Calibrate Sensor
   Add On/Off switch and function
   Add On/Off LED indicator

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
   
 */

#include <NewPing.h> // Include NewPing to handle ultraSensor data
#include <DRV8833.h> // Include DRV8833 Dual Motor Driver Carrier - Pololu  

const int ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const int ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100; // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
const int inputA1 = 11;   //PWM control Right Motor -
const int inputA2 = 10;   //PWM control Right Motor +
const int inputB1 = 5;  //PWM control Left Motor +
const int inputB2 = 6;  //PWM control Left Motor -

int instruction = 0; //Initial Command

int motorSpeed;
int dutyCycle;

DRV8833 driver = DRV8833(); // Create an instance of the DRV8833:
NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.

void setup(){
  Serial.begin(9600);             //Start Serial connection
/* Attach the motors to the input pins: */
  driver.attachMotorA(inputA1, inputA2);
  driver.attachMotorB(inputB1, inputB2);
  
  Serial.println("Ready!");
}

void loop(){
 
  delay(29);                      // Wait 29ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
  unsigned int ultraSensorRaw = ultraSensor.ping(); // Send ping, get ping time in microseconds (uS).
  int ultraSensorCM = ultraSensorRaw / US_ROUNDTRIP_CM; // Convert ping time to distance in cm and print result (0 = outside set distance range)

/* Print to Serial ultraSensorCM */
  Serial.print("Ping: ");
  Serial.print(ultraSensorCM); 
  Serial.println("cm");

/* Hamster checks if ultraSensor sees anything 30 cm in front of it, and rotates right if it does. Otherwise, Hamster drives forward.*/
  if(ultraSensorCM > 0 && ultraSensorCM <= 30){
    instruction = 3; // rotate right
  } else {
    instruction = 1; // go forwards
  }

/* Map motorSpeed (255 - 0) to dutyCycle (0 - 100)*/
  dutyCycle = 50;
  motorSpeed = map(dutyCycle, 0, 100, 0, 255);
  Serial.print("Duty Cycle: ");
  Serial.print(dutyCycle); 
  Serial.println("%");
  
/* Motor control */
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
  delay(50); // Continue instruction for +50ms (79ms total)
}  

