/*

Matrix Keyboard
(c) Martin Gritzan, 2017

*/

#define NUM_ROWS 4
#define NUM_COLS 4

// interface pins for matrix display
const int rowPins[NUM_ROWS] = {22, 24, 26, 28};
const int colPins[NUM_COLS] = {30, 32, 34, 36};
const char keycodes[NUM_ROWS][NUM_COLS]= {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

void setup() {
  
  Serial.begin(9600);
  
  for (int i = 0; i < NUM_ROWS; ++i)
  {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }
  for (int i = 0; i < NUM_COLS; ++i)
    pinMode(colPins[i], INPUT_PULLUP);  
}

void loop() {
  for (int row = 0; row < NUM_ROWS; row++)
  {
    digitalWrite(rowPins[row], LOW);
    for (int col = 0; col < NUM_COLS; col++)
      if (digitalRead(colPins[col]) == LOW)
      {
        Serial.print(keycodes[row][col]);
        while(digitalRead(colPins[col]) == LOW);
      }
    digitalWrite(rowPins[row], HIGH);
  }
  
}
