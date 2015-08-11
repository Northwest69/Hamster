Hamster
=======
Firmware: 1.2.0 
Created by Peter Chau
Start Date: June 5, 2015

Hamster checks if ultraSensor sees anything 40 cm in front of it. If it does, it chooses an random action based on a set of probabilities. Otherwise, Hamster drives forward. 

When Hamster is in 'Learning Mode', it evaluates it's actions and modifies the probability set until it's tried 100 times. The blue light is on when it's in 'Learning Mode'!

When Hamster is in 'Roam Mode', it uses the probability set to avoid objects!

When Hamster is in 'Standby Mode', it does nothing until it sees something. When it does see something, it enters 'Roam Mode'

Data is sent to PC via bluetooth serial terminal.

Hardware: Arduino Uno, TI DRV8833 Dual H-Bridge Motor Driver, HC-SR04 Ultra01 + Ultrasonic Range Finder, Blue tooth Shield HC-06, HMC5883L triple axis compass

To do:
------
   Read/Write probabilities to computer
   Add probability stream to computer display
   When I stop giving it a processing command, it doesn't return to non-slave mode
   
Log:
----
###8/11/2015 1.3.0
	Arduino can handle set_speed, Max learning attempts, reset learning attempts, and rotate degrees


###8/10/2015 1.2.0
	Now using SerialCommand Library for string and argument based serial communication
		"D 0\r" = Drive forwards
	Fixed Standby and Roam modes

###8/9/2015 1.1.1
	Created Processing software for Hamster
	Allow Arduino to Receive Commands via bluetooth
		Can control driving instructions for drive train
			via on screen buttons
			via keys (up down left right control (stop))
	Configured Bluetooth to communicate at 38400 BAUD
		Optimized timing  to 1 loop per 50ms

###8/9/2015 1.0.1
	Memory optimizations
		Use Flash memory for strings
			Read from strings from memory for better performance (save the string only 1 time);
		Reduced code appearances for printing current probabilities more than once
		Use bytes, and boolean data types when possible

###8/8/2015 1.0.0
   Add mode switch (Learning, Standby, Roam)
   		Implemented all 3 modes
   		
###8/6/2015
	Created a prototype shield
	Power LED
	On/Off Switch

###8/4/2015 0.3.4
   Cleaned up and optimized code

###7/30/2015 0.3.3
   Implemented rotate to a degree compass navigation
      Hard-coded degrees to drive train
	  It rotates a fixed amount then stops when it's clear
   Stop motors when we see something	

###7/29/2015 0.3.2
   Enable triple axis compass
   Implemented compass navigation to drive straight
	   Added buffer so no adjustment occurs when heading difference is less than 1 degree 

###7/27/2015
   Added a probability floor

###7/22/2015
   Changed name of bluetooth module to: Hamster
   Changed Password to: Swag
   Developed Bluetooth Config Sketch

###7/21/2015
   Add Bluetooth functionality via HC-06 Bluetooth Shield
	  Sends data to computer
	  Formatted Serial data
   Changed Colors for better aesthetic
   Implement software serial for bluetooth

###7/15/2015
   Added Debounce 2 library to add functionality to debounce buttons. Perhaps this can be used to buffer actions
   Test Hamster at learningAttempts = 100, increased to 500. This works very well.
   Add exit AAN if a probability threshold is reached

###7/13/2015
   The issue was the USB cable. It is now fixed!
   Changed neural network adjustments to small decreases and increases (from 100% to 1%, and 33% to 0.33%)
   Add max training attempts (50)
     Add exit if n training attempts reached
   Add EEPROM library
     Update probabilities to EEPROM
     Nevermind... blew max writes on mem 0 already. Fuck that. Will wait for bluetooth communications
   Now using 5 different actions (stop, forwards, backwards, rotate right, rotate left)
   Add active Feedforward Back Propagation Neural Network
     Add "training mode"
   Add LED to indication which mode Hamster is in

###7/12/2015
   Added a Feedforward Neural Network for obstacle avoidance
   USB drive on computer mangled again.... Have thought out robot more. Very happy with current results.

###7/10/2015
   Ultrasonic range finder working well, however there is lag between when it detects an object and when it takes action.
     Calibrated Sensor
   Poor wheel alignment
   Implemented a status LED

###7/9/2015
   Create DC Motor drive function. void driveTrain(int instruction, int dutyCycle, int period)
   Implemented driveTrain()
   Created better serial messages
   Reworked ultrasonic range finder pinging as a state function
   ultrasonic range find needs to be calibrated. wtf. it's slow.
   ok now the robot works but is really dumb lol. it runs into stuff... lag time... momentum... go and :speed up ping

###7/8/2015
   Arduino Uno now functioning - Window's driver needed to be installed. Gave it a fresh battery charge as well.
   Updated Breadboard and schematic - was missing Uno VIN.

###7/7/2015
   Created Breadboard Diagram
   Created Schematic

###7/6/2015
   Installed DVR8833 library
   Mapped dutyCycle to motorSpeed
   I seem to have fired my Arduino UNO. Computer won't recognize it however when plugged into the wall via USB, the board works fine.

Functions:
==========

###void driveTrain(int instruction, int dutyCycle, float currentHeading)
Instructions
   0   Forwards
   1   Backwards
   2   Rotate right by degree
   3   Rotate left by degree
   4   Rotate right
   5   Rotate left
   6   Stop
Duty Cycle
   0% - 100%

###void statusLED(int status)
Statuses
   0   Green   Ready
   1   Red   Object Avoidance
   2   Blue   Action Success
   3   Purple   Wander
   4   Light Blue   Ping
   5   Light Red    Action Failed

###int weightedRandom(float* weights)
Weights
   Action Probabilities
