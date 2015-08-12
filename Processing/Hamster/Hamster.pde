/* Hamster
   Software: 0.3.0
   Created by Peter Chau
   Start Date: August 9, 2015
*/

import processing.serial.*;
import controlP5.*; // For GUI interface

Serial myPort;
ControlP5 myControls;

/* Buttons */
int[] forward = {150, 0};
int[] backward = {150, 300};
int[] rotateRight = {300, 150};
int[] rotateLeft = {0, 150};
int[] stop = {150, 150};
int boxSize = 100;
boolean[] overBox = {false, false, false, false, false};

/* Slider */ 
public int dutyCycle = 75;
public float rotateDegree = 30;
int[] dutyCycleSlider = {450, 50};
int[] rotateDegreeSlider = {500, 50};

void setup(){
  String portName = Serial.list()[1]; //1 is bluetooth, 2 is serial
  myPort = new Serial(this, portName, 38400);
  
  size(600, 500);
  background(100);
  
  /* GUI interface */
  mySliders = new ControlP5(this);
  myControls.addSlider("dutyCycle", 0, 100, dutyCycle, dutyCycleSlider[0], dutyCycleSlider[1], 10, 100);
  myControls.addSlider("rotateDegree", 1, 360, rotateDegree, rotateDegreeSlider[0], rotateDegreeSlider[1], 10, 100);
  }

void draw() {
  background(100);
  
  /* Draw buttons */
  rect(forward[0], forward[1], boxSize, boxSize); 
  rect(backward[0], backward[1], boxSize, boxSize);  
  rect(rotateRight[0], rotateRight[1], boxSize, boxSize);   
  rect(rotateLeft[0], rotateLeft[1], boxSize, boxSize);   
  rect(stop[0], stop[1], boxSize, boxSize);
  
  /* Check cursor location */
  if (mouseX > forward[0] && mouseX < forward[0]+boxSize && mouseY > forward[1]-boxSize && mouseY < forward[1]+boxSize){
    overBox[0] = true;
  } else {
    overBox[0] = false;
  }
  if (mouseX > backward[0] && mouseX < backward[0]+boxSize && mouseY > backward[1] && mouseY < backward[1]+boxSize){
    overBox[1] = true;;    
  } else {
    overBox[1] = false;    
  }
  if (mouseX > rotateRight[0] && mouseX < rotateRight[0]+boxSize && mouseY > rotateRight[1] && mouseY < rotateRight[1]+boxSize){
    overBox[2] = true;  
  } else {
    overBox[2] = false;
  }
  if (mouseX > rotateLeft[0] && mouseX < rotateLeft[0]+boxSize && mouseY > rotateLeft[1] && mouseY < rotateLeft[1]+boxSize){
    overBox[3] = true;
  } else {
    overBox[3] = false;
  }
  if (mouseX > stop[0] && mouseX < stop[0]+boxSize && mouseY > stop[1] && mouseY < stop[1]+boxSize){
    overBox[4] = true;
  } else {
    overBox[4] = false;
  }
 
/* Send command button pressed */
if(mousePressed){
  if(overBox[0]){
    myPort.write("D 0\r");
    println("D 0\r");
  } else if(overBox[1]) {
    myPort.write("D 1\r");
    println("D 1\r");     
  } else if(overBox[2]) {
    myPort.write("D 4\r");
    println("D 4\r");    
  } else if(overBox[3]) {
    myPort.write("D 5\r");
    println("D 5\r");      
  } else if(overBox[4]) {
    myPort.write("D 6\r");
    println("D 6\r");      
  } 
}

/* Send command if keyboard button pressed */
if (keyPressed == true){
if (key == CODED){
  if(keyCode == UP){
    myPort.write("D 0\r");
    println("D 0\r");
  } else if(keyCode == DOWN) {
    myPort.write("D 1\r");
    println("D 1\r");     
  } else if(keyCode == RIGHT) {
    myPort.write("D 4\r");
    println("D 4\r");   
  } else if(keyCode == LEFT) {
    myPort.write("D 5\r");
    println("D 5\r");      
  } else if(keyCode == CONTROL) {
    myPort.write("D 6\r");
    println("D 6\r");   
  } 
} else if(key == ENTER) {
    myPort.write("S " + dutyCycle + "\r");
    println("S " + dutyCycle + "\r");  
    myPort.write("I " + rotateDegree + "\r");
    println("I " + rotateDegree + "\r");   
  }
}

}
