/*

Matrix Keyboard
(c) Martin Gritzan, 2017

*/

#define NUM_ROWS 4
#define NUM_COLS 4

// interface pins for matrix display
const int rowPins[NUM_ROWS] = {12, 11, 10, 9};
const int colPins[NUM_COLS] = {8, 7, 6, 5};

void setup_keyboard(){
  for (int i = 0; i < NUM_ROWS; ++i)
  {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }
  for (int i = 0; i < NUM_COLS; ++i)
    pinMode(colPins[i], INPUT_PULLUP);
}

char poll_keyboard() {
  const char keycodes[NUM_ROWS][NUM_COLS]= {
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' }
  };

  for (int row = 0; row < NUM_ROWS; row++)
  {
    digitalWrite(rowPins[row], LOW);
    for (int col = 0; col < NUM_COLS; col++)
      if (digitalRead(colPins[col]) == LOW)
      {
        while(digitalRead(colPins[col]) == LOW);
        return keycodes[row][col];
      }
    digitalWrite(rowPins[row], HIGH);
  }
  return -1;
}

void setup() {
  Serial.begin(9600);
  setup_keyboard();
}

void loop() {
  char keycode = poll_keyboard();
  if (keycode != -1)
    Serial.print(keycode);
}
