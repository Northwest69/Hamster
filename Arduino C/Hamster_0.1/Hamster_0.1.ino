/* Hamster 0.1 created by Peter Chau
   June 5, 2015
   
   Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder
   
   Hamster checks if ultraSensor sees anything 30 cm in front of it, and quickly rotates right if it does. Otherwise, Hamster drives forward slowly.
   It's scared of things!
    
   To do:

   PWM seems to be to have 0 as the highest value
   Make Hamster look around then determine the best direction to drive
   Create DC Motor drive function
   Calibrate Sensor
   Add On/Off switch and function
   Add On/Off LED indicator
   Remap PWM
   
 */

#include <NewPing.h> // Include NewPing to handle ultraSensor data

const int ultraSensorTriggerPin = 12;  // Arduino pin tied to trigger pin on the ultrasonic sensor.
const int ultraSensorEchoPin = 13;  // Arduino pin tied to echo pin on the ultrasonic sensor.
const int maxDistance = 100; // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.
const int rMotorN = 5;   //PWM control Right Motor -
const int rMotorP = 6;   //PWM control Right Motor +
const int lMotorP = 10;  //PWM control Left Motor +
const int lMotorN = 11;  //PWM control Left Motor -

int instruction = 0; //Initial Command

int motorSpeed;
int dutyCycle;


NewPing ultraSensor(ultraSensorTriggerPin, ultraSensorEchoPin, maxDistance); // NewPing setup of pins and maximum distance.

void setup(){
  pinMode(rMotorN, OUTPUT);  //Set control pins to be outputs
  pinMode(rMotorP, OUTPUT);
  pinMode(lMotorP, OUTPUT);
  pinMode(lMotorN, OUTPUT);

  digitalWrite(rMotorN, LOW);  //Set both motors off for start-up
  digitalWrite(rMotorP, LOW);
  digitalWrite(lMotorP, LOW);
  digitalWrite(lMotorN, LOW);

  Serial.begin(9600);             //Start Serial connection
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
  dutyCycle = 0;
  motorSpeed = map(dutyCycle, 0, 100, 0, 255);
  Serial.print("Duty Cycle: ");
  Serial.print(dutyCycle); 
  Serial.println("%");
  
/* Motor control */
  switch(instruction){
     case 0:
       digitalWrite(rMotorN, LOW);  
       digitalWrite(rMotorP, LOW);
       digitalWrite(lMotorP, LOW);  
       digitalWrite(lMotorN, LOW);
       Serial.println("Stop\n");
     break;

      case 1:
       analogWrite(rMotorN, dutyCycle);  // PWM reverse polarity?
       digitalWrite(rMotorP, HIGH);
       analogWrite(lMotorN, dutyCycle);  
       digitalWrite(lMotorP, HIGH);
       Serial.println("Forward\n");
     break;
     
     case 2:
       digitalWrite(rMotorN, HIGH);  
       analogWrite(rMotorP, dutyCycle);
       digitalWrite(lMotorN, HIGH);  
       analogWrite(lMotorP, dutyCycle);
       Serial.println("Backward\n");
     break; 
         
     case 3:
       digitalWrite(rMotorP, HIGH); 
       analogWrite(rMotorN, dutyCycle);
       analogWrite(lMotorP, dutyCycle);  
       digitalWrite(lMotorN, HIGH);
       Serial.println("Rotate Right\n");
     break;
     
       case 4:
       analogWrite(rMotorP, dutyCycle);  
       digitalWrite(rMotorN, HIGH);
       digitalWrite(lMotorP, HIGH);  
       analogWrite(lMotorN, dutyCycle);
       Serial.println("Rotate Left\n");
     break;     
  }
  delay(100); // Continue instruction for +100ms (129ms total)
}  

