Hamster 0.2.0 
Created by Peter Chau
Start Date: June 5, 2015
Project: Alpha

Hamster checks if ultraSensor sees anything 40 cm in front of it. If it does, it chooses an random action based on a set of probabilities. Otherwise, Hamster drives forward. 

When Hamster is in 'Learning Mode', it evaluates it's actions and modifies the probability set until it's tried 100 times. The blue light is on when it's in 'Learning Mode'!

Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder

To do:
Add mode switch
Install compass
Add On/Off switch and function
Add On/Off LED indicator
Make hamster drive staight
Make Hamster look around then determine the best direction to drive
Add exit AAN if a probability threshold is reached
Create lower limit for probabilities

Log:
7/15/2015
Added Debounce 2 library to add functionallity to debounce buttons. Perhaps this can be used to buffer actions
Test Hamster at learningAttempts = 100, increased to 500. This works very well.

7/13/2015
The issue was the USB cable. It is now fixed!
Changed neural network adjustments to small decreases and increases (from 100% to 1%, and 33% to 0.33%)
Add max training attempts (50)
  Add exit if n training attempts reached
Add EEPROM library
  Update probabilities to EEPROM
  Nevermind... blew max writes on mem 0 already. Fyuck that. Will wait for bluetooth communications
Now using 5 differant actions (stop, forwards, backwards, rotate right, rotate left)
Add active Feedforward Back Proprogation Neutal Network
  Add "training mode"
  Add LED to indication which mode Hamster is in

7/12/2015
Added a Feedforward Neural Network for obstacle avoidance
USB drive on computer mangled again.... Have thought out robot more. Very happy with current results.

7/10/2015
Ultrasonic rangefinder working well, however there is lag between when it detects an object and when it takes action.
  Calibrated Sensor
Poor wheel alignment
Implemented a status LED

7/9/2015
Create DC Motor drive function. void driveTrain(int instruction, int dutyCycle, int period)
Implemented driveTrain()
Created better serial messages
Reworked ultrasonic rangefinder pinging as a state function
ultrasonic rangefind needs to be calibrated. wtf. it's slow.
ok now the robot works but is really dumb lol. it runs into stuff... lag time... momentum... go and :speed up ping

7/8/2015
Arduino Uno now functioning - Window's driver needed to be installed. Gave it a fresh battery charge aswell.
Updated Breadboard and schematic - was missing Uno VIN.

7/7/2015
Created Breadboard Diagram
Created Schematic

7/6/2015
Installed DVR8833 library
Mapped dutyCycle to motorSpeed
I seem to have fired my Arduino UNO. Computer won't recognize it however when plugged into the wall via USB, the board works fine.

Functions

void driveTrain(int instruction, int dutyCycle)
	Instructions
		0	Stop
		1	Forwards
		2	Backwards
		3	Rotate right
		4	Rotate left

	Duty Cycle
		0% - 100%

void statusLED(int status)
	Wtatuses
		0	Green		Ready
		1	Red 		Object Avoidance
		2	Blue 		Wander
		3	Purple		Action Success
		4	Light Blue 	Ping
		5	Light Red 	Action Failed

int weightedRandom(float* weights)
	Weights
		Action Probabilities
