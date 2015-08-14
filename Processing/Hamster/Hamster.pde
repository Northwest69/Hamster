/* Hamster
 Software: 0.5.7645
 Created by Peter Chau
 Start Date: August 9, 2015
 */

import processing.serial.*;
import controlP5.*; // For GUI interface

Serial myPort;

// GUI Declarations
ControlP5 myControls;
Slider dutySlider, degreeSlider;
Textfield attemptsTextfield, maxTextfield;

/* Slider */
public int dutyCycle = 75;
public float rotateDegree = 30;

int[] dutyCycleSlider = {
  250, 50
};
int[] rotateDegreeSlider = {
  300, 50
};

/* Textfields */
String attempts = "0";
String maxAttempts = "500";
int[] learningAttempt = {
  400, 100
};
int[] maxAttempt = {
  400, 50
};

/* Button */
int[] forward = {
  100, 50
};
int[] backward = {
  100, 150
};
int[] left = {
  50, 100
};
int[] right = {
  150, 100
};
int[] stop = {
  100, 100
};
int[] set = {
  400, 150
};
int[] reset = {
  400, 175
};

void setup() {
  String portName = Serial.list()[1]; //1 is bluetooth, 2 is serial
  myPort = new Serial(this, portName, 38400);
  myPort.clear();

  size(800, 250);
  background(100);

  /* GUI interface */
  myControls = new ControlP5(this);
  dutySlider = myControls.addSlider("dutyCycle", 0, 100, dutyCycle, dutyCycleSlider[0], dutyCycleSlider[1], 10, 137);
  degreeSlider = myControls.addSlider("rotateDegree", 1, 360, rotateDegree, rotateDegreeSlider[0], rotateDegreeSlider[1], 10, 137);
  maxTextfield = myControls.addTextfield("Max Attempts").setPosition(maxAttempt[0], maxAttempt[1]).setText(maxAttempts).setSize(50, 25).setAutoClear(false);
  attemptsTextfield = myControls.addTextfield("Current Attempts").setPosition(learningAttempt[0], learningAttempt[1]).setText(attempts).setSize(50, 25).setAutoClear(false);

  myControls.addButton("Forward")
    .setPosition(forward[0], forward[1])
    .setSize(50, 50)
    .setValue(100)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("D 0\r");
        println("D 0\r");
      }
    }
  }
  );  
  myControls.addButton("Backward")
    .setPosition(backward[0], backward[1])
    .setSize(50, 50)
    .setValue(100)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("D 1\r");
        println("D 1\r");
      }
    }
  }
  ); 
  myControls.addButton("Right")
    .setPosition(right[0], right[1])
    .setSize(50, 50)
    .setValue(100)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("D 4\r");
        println("D 4\r");
      }
    }
  }
  ); 
  myControls.addButton("Left")
    .setPosition(left[0], left[1])
    .setSize(50, 50)
    .setValue(100)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("D 5\r");
        println("D 5\r");
      }
    }
  }
  ); 
  myControls.addButton("Stop")
    .setPosition(stop[0], stop[1])
    .setSize(50, 50)
    .setValue(100)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("D 6\r");
        println("D 6\r");
      }
    }
  }
  ); 
  myControls.addButton("Set")
    .setPosition(set[0], set[1])
    .setSize(50, 25)
    .setValue(0)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("S " + dutyCycle + "\r");
        println("S " + dutyCycle + "\r");  
        myPort.write("T " + rotateDegree + "\r");
        println("T " + rotateDegree + "\r");
        maxAttempts = myControls.get(Textfield.class, "Max Attempts").getText();
        myPort.write("L " + maxAttempts + "\r");
        println("L " + maxAttempts + "\r");
      }
    }
  }
  );  
  myControls.addButton("Reset")
    .setPosition(reset[0], reset[1])
    .setSize(50, 25)
    .setValue(0)
    .addCallback(new CallbackListener() {
    public void controlEvent(CallbackEvent event) {
      if (event.getAction() == ControlP5.ACTION_PRESSED) {
        myPort.write("R\r");
        println("R\r");
        attempts = "0";
        dutyCycle = 75;
        rotateDegree = 30;
        maxAttempts = "500";
        attemptsTextfield.setValue(attempts);
        dutySlider.setValue(dutyCycle);
        degreeSlider.setValue(rotateDegree);
        maxTextfield.setValue(maxAttempts);
      }
    }
  }
  );
}

void draw() {
  background(100);

  /* Send command if keyboard button pressed */
  if (keyPressed == true) {
    if (key == CODED) {
      if (keyCode == UP) {
        myPort.write("D 0\r");
        println("D 0\r");
      } else if (keyCode == DOWN) {
        myPort.write("D 1\r");
        println("D 1\r");
      } else if (keyCode == RIGHT) {
        myPort.write("D 4\r");
        println("D 4\r");
      } else if (keyCode == LEFT) {
        myPort.write("D 5\r");
        println("D 5\r");
      } else if (keyCode == CONTROL) {
        myPort.write("D 6\r");
        println("D 6\r");
      }
    } else if (key == ENTER) {
      myPort.write("S " + dutyCycle + "\r");
      println("S " + dutyCycle + "\r");  
      myPort.write("T " + rotateDegree + "\r");
      println("T " + rotateDegree + "\r");
      maxAttempts = myControls.get(Textfield.class, "Max Attempts").getText();
      myPort.write("L " + maxAttempts + "\r");
      println("L " + maxAttempts + "\r");
    }
  }

  while (myPort.available () > 0) {
    String current = myPort.readStringUntil('\n');
    // if you got any bytes other than the linefeed:
    if (current != null) {
      current = trim(current);
      String[] currentString = split(current, " ");
      if (currentString[0].equals("C")) {
        attempts = currentString[1];
        attemptsTextfield.setValue(attempts);
        println(currentString[0]);
      }
    }
  }
}