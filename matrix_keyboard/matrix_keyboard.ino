/*

Matrix Keyboard
(c) Martin Gritzan, 2017

*/

// interface pins for matrix display
const int rowPins[] = {20, 22, 24, 26};
const int colPins[] = {28, 30, 32, 34};

void setup() {
  
  Serial.begin(9600);
  
  for (int i = 0; i < 4; ++i) {
    pinMode(rowPins[i], OUTPUT);
    pinMode(colPins[i], INPUT);
  }
  
}

void loop() {
  
  
}
