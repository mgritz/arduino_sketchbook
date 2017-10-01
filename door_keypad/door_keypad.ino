/*

Door Keypad for primitive house alarm.
(c) Martin Gritzan, 2017

*/

//##################################################
//################# Keypad stuff ###################
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
        Serial.print('[');
        Serial.print(keycodes[row][col]);
        Serial.print("]\n");
        return keycodes[row][col];
      }
    digitalWrite(rowPins[row], HIGH);
  }
  return -1;
}

//##################################################
//################# Status LEDs ####################
const int sled_rd = 4;
const int sled_ye = 3;

void setup_status_leds() {
  pinMode(sled_rd, OUTPUT);
  pinMode(sled_ye, OUTPUT);
}

void flash_led(int led, int code, int interval) {
  for(int i = 0; i < code; ++i)
  {
    digitalWrite(led, HIGH);
    delay(interval);
    digitalWrite(led, LOW);
    if(i < code - 1)
      delay(interval);
  }
}

unsigned long next_flash;
const unsigned long IDLE_FLASH_PERIOD = 3000;
const unsigned long ARMED_FLASH_PERIOD = 1500;

//##################################################
//################# Keypad State Machine ###########
typedef enum {
  IDLE_DISARMED,
  IDLE_ARMED,
  DOOR_OPENED,
  KEY_ENTERED,
  CHECKING_KEY
} states_type;

states_type Door_state;

//##################################################
//################# SETUP ##########################
void setup() {
  Serial.begin(9600);
  setup_keyboard();
  setup_status_leds();
  Door_state = IDLE_DISARMED;
  Serial.print("setup complete\n");
  Serial.print("state IDLE_DISARMED\n");
}
//##################################################
//################# RUN ############################
void loop() {
  
  switch(Door_state){
    // Disarmed state. Blink LED from time to time
    // and wait for input.
    case IDLE_DISARMED:{
      char keycode = poll_keyboard();
      
      // arm alarm
      if (keycode == 'A'){

        // TODO check if server is connected.

        if(true)
        {
          Door_state = IDLE_ARMED;
          Serial.print("state IDLE_ARMED\n");
          flash_led(sled_ye, 3, 100);
        } else {
          flash_led(sled_ye, 8, 50);
          flash_led(sled_rd, 3, 100);
        }
      }

      // blink status led
      if (millis() > next_flash)
      {

        // TODO periodically check if server is connected.

        if(true)
          flash_led(sled_rd, 1, 250);
        else
          flash_led(sled_rd, 3, 100);
        next_flash = millis() + IDLE_FLASH_PERIOD;
      }
    }
    break;
    
    case IDLE_ARMED:{
      
    }
    break;
    
    case DOOR_OPENED:{
      
    }
    break;
    
    case KEY_ENTERED:{
      
    }
    break;
    
    case CHECKING_KEY:{
      
    }
    break;
  }
}
