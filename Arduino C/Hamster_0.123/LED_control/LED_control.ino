const int redLedPin = 4;
const int greenLedPin = 2;
const int blueLedPin = 1;

int status = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  }

void loop() {
  statusLed(status); 
}

void statusLed(int status){
  switch(status){
  case 0: // set to LED green to indicate status
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW); 
  if(digitalRead(blueLedPin) != HIGH) {
    digitalWrite(blueLedPin, HIGH);
  }
  break;  
  case 1: // set to LED red to indicate status
  digitalWrite(greenLedPin, LOW);
  digitalWrite(blueLedPin, LOW); 
  if(digitalRead(redLedPin) != HIGH) {
    digitalWrite(redLedPin, HIGH);
  }
  break;

  case 2: // set to LED blue to indicate status
  digitalWrite(redLedPin, LOW);
  digitalWrite(blueLedPin, LOW); 
  if(digitalRead(greenLedPin) != HIGH) {
    digitalWrite(greenLedPin, HIGH);
  }
  break;

  case 3: // set to LED purple to indicate status
  digitalWrite(blueLedPin, LOW); 
  if(digitalRead(greenLedPin) != HIGH) {
    digitalWrite(greenLedPin, HIGH);
  }
  if(digitalRead(redLedPin) != HIGH) {
    digitalWrite(redLedPin, HIGH);
  }
  break; 
  case 4: // set to LED light blue to indicate status
  digitalWrite(redLedPin, LOW); 
  if(digitalRead(greenLedPin) != HIGH) {
    digitalWrite(greenLedPin, HIGH);
  }
  if(digitalRead(blueLedPin) != HIGH) {
    digitalWrite(blueLedPin, HIGH);
  }
  break; 
  case 5: // set to LED light red to indicate status
  digitalWrite(greenLedPin, LOW); 
  if(digitalRead(redLedPin) != HIGH) {
    digitalWrite(redLedPin, HIGH);
  }
  if(digitalRead(blueLedPin) != HIGH) {
    digitalWrite(blueLedPin, HIGH);
  }
  break; 
  }
}

