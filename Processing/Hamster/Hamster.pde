/* Hamster
   Software: 0.1.1
   Created by Peter Chau
   Start Date: August 9, 2015
*/

import processing.serial.*;
Serial myPort;

/* Buttons */
int[] forward = {150, 0};
int[] backward = {150, 300};
int[] rotateRight = {300, 150};
int[] rotateLeft = {0, 150};
int[] stop = {150, 150};
int boxSize = 100;
boolean[] overBox = {false, false, false, false, false};
 
/* Colors */
color button = #F00000;
color buttonHover = #A00000;
color buttonActive = #400000;
color backgroundColor = #ffffff;
color outline = #000000;

void setup(){
  String portName = Serial.list()[1]; //1 is bluetooth, 2 is serial
  myPort = new Serial(this, portName, 38400);
  
  size(400, 400);
  background(backgroundColor);
  
  /* Draw buttons */
  stroke(outline);  
  fill(button);  
  rect(forward[0], forward[1], boxSize, boxSize);
  fill(button);
  stroke(outline);    
  rect(backward[0], backward[1], boxSize, boxSize);
  fill(button);
  stroke(outline);    
  rect(rotateRight[0], rotateRight[1], boxSize, boxSize);
  fill(button);
  stroke(outline);    
  rect(rotateLeft[0], rotateLeft[1], boxSize, boxSize);
  fill(button);
  stroke(outline);    
  rect(stop[0], stop[1], boxSize, boxSize);
}

void draw() {

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
    myPort.write(0);
    println('0');
  } else if(overBox[1]) {
    myPort.write(1);
    println('1');     
  } else if(overBox[2]) {
    myPort.write(4);
    println('4');    
  } else if(overBox[3]) {
    myPort.write(5);
    println('5');      
  } else if(overBox[4]) {
    myPort.write(6);
    println('6');      
  }
}

/* Send command if keyboard button pressed */
if (key == CODED){
  if(keyCode == UP){
    myPort.write(0);
    println('0');
  } else if(keyCode == DOWN) {
    myPort.write(1);
    println('1');     
  } else if(keyCode == RIGHT) {
    myPort.write(4);
    println('4');    
  } else if(keyCode == LEFT) {
    myPort.write(5);
    println('5');      
  } else if(keyCode == CONTROL) {
    myPort.write(6);
    println('6');      
  }
}

}
