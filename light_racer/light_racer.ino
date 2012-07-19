/*    
  Version 2 
  16 lights run from one end to the other
  button stops and blinks current light
  pot sets an overal skill multiplier (which is shown temporarily if changed)  
  */ 

const int rowPins[] = { 9, 8, 7, 6, 5, 4, 3, 2}; 
const int columnPins[]   = { 12, 11}; 
const int potPin = 0; 
const int buttonPin = 13;

int buttonState = 0;
int potVal = -1;              
int pixel       = 0;        // 0 to 63 LEDs in the matrix 
int columnToLight = 0;        // pixel value converted into LED column 
int rowToLight    = 0;        // pixel value converted into LED row 

int level         = 0;
int misses        = 0;

void setup() {   
  Serial.begin(9600);
  
  for (int i = 0; i < 2; i++) {
    pinMode(columnPins[i], OUTPUT);  // make all the LED pins outputs  
  }
  for (int i = 0; i < 8; i++) {  
    pinMode(rowPins[i], OUTPUT);   
  } 
  
  blinkLevelOn(level);
} 

void loop() {
  light(pixel);
  
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {     
    for (int i = 0; i < 5; i++) {
      // 5 blinks on the current state
      digitalWrite(rowPins[rowToLight], HIGH);
      delay(300);
      digitalWrite(rowPins[rowToLight], LOW);
      delay(300);
    }
    if (pixel == 15) {
      level += 1;
      Serial.print("level up:"); Serial.println(level);
      misses = 0;
      if (level == 16) {
        lightShow();
        level = 15;
      } else {
        blinkLevelOn(level);
      }
    } else {
      misses += 1;
      if (misses > 2 && level > 0) {
        level -= 1;
        misses = 0;
        Serial.print("level down:"); Serial.println(level);
        blinkLevelOn(level);
      } 
    }
    pixel = 0;
  } else {
    pixel += 1;
    if(pixel > 15) { pixel = 0; }
  }
  
  // to increase general difficulty add more to potVal and adjust the high end of maper
  int speed = map((potVal+15)*level, 0, 450, 140, 0);
  // Serial.print("pot:"); Serial.print(potVal); Serial.print(" level:"); Serial.print(level); Serial.print(" speed:"); Serial.println(speed);
  // TODO: button won't be detected if pressed during delay, ideal to change this to while loop
  delay(speed);
  
  for(int row = 0; row < 8; row++) {
    digitalWrite(rowPins[row], LOW);
  }
  
  checkPot();
}

boolean checkPot(){
  // SET THE MULTIPLIER
  int tpot = map(analogRead(potPin), 0, 1000, 0, 15);
  if (potVal == -1) {potVal = tpot;} // keeps the initial value from being shown 
  if (tpot != potVal){
    //if tpot is new, show the new value;
    potVal = tpot; 
    showLightBar(1, tpot, 300);
    return true; 
  }
  return false;
}

void light(int pixel){
  if (pixel >= 8){
   digitalWrite(columnPins[0], HIGH);
   digitalWrite(columnPins[1], LOW);
  } else {
   digitalWrite(columnPins[0], LOW);
   digitalWrite(columnPins[1], HIGH);
  }
  
  if (pixel < 8) {
    rowToLight = pixel;
  } else {
    rowToLight = pixel-8;
  }  
  for(int row = 0; row < 8; row++) {
    if (row==rowToLight){
      digitalWrite(rowPins[row], HIGH);
    } else {
      digitalWrite(rowPins[row], LOW);
    }
  }
}

void unlight(){
  for(int col = 0; col < 2; col++) {
    digitalWrite(columnPins[col], HIGH);
  }
  for(int row = 0; row < 8; row++) {
    digitalWrite(rowPins[row], LOW);
  }
}

void blinkLevelOn(int level){
  unlight();
  for (int i = 0; i<level; i++){
    showLightBar(1, i, 10);
  }
  for (int i = 0; i<3; i++){
    showLightBar(1, level-1, 20);
    showLightBar(1, level, 20);
  }
  showLightBar(1, level, 100);
  for (int i = level; i>0; i--){
    showLightBar(1, i, 10);
  }
  delay(200);
}

void lightShow(){
  for (int i=0; i<2; i++){
    unlight();
    for (int i = 0; i<15; i++){
      showLightBar(1, i, 5);
    }
    unlight();
    for (int i = 0; i<15; i++){
      showLightBar(-1, i, 5);
    }
  }
  for (int i = 0; i<100; i++){
    light(random(0, 16));
    delay(40);
    unlight();
  }
  for (int i=0; i<1; i++){
    unlight();
    for (int i = 0; i<15; i++){
      showLightBar(1, i, 5);
    }
    unlight();
    for (int i = 0; i<15; i++){
      showLightBar(-1, i, 5);
    }
  }
  delay(400);
}

void showLightBar(int dir, int numLights, int duration) {
  int columnLevel = numLights / 8;                     // map to the number of columns
  int rowLevel = numLights % 8;        // get the fractional value
  
  // TODO: ideal to make this time based rather than step based
  for (int i = 0; i<duration; i++){
    boolean changed = checkPot();
    if (changed) { break; }
    for (int column = 0; column < 2; column++) {
      digitalWrite(columnPins[column], LOW);     // connect this column to Ground
      for(int row = 0; row < 8; row++) {
        if (columnLevel > column && dir == 1 || columnLevel < column && dir == -1 ) {
          digitalWrite(rowPins[row], HIGH);  // connect all LEDs in row to +5 volts
        } else if (columnLevel == column && rowLevel >= row && dir == 1 || columnLevel == column && rowLevel <= row && dir == -1) {
          digitalWrite(rowPins[row], HIGH);
        } else {
          digitalWrite(columnPins[column], LOW); // turn off all LEDs in this row
        } 
        delayMicroseconds(300);     // delay gives frame time of 20ms for 64 LEDs
        digitalWrite(rowPins[row], LOW);         // turn off LED
      }
      digitalWrite(columnPins[column], HIGH);   // disconnect this column from Ground
    }
  }
}
