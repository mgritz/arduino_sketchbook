#include <LowLevel.h>
#include <OneWireSlave.h>

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
const unsigned long INTRUDER_FLASH_PERIOD = 500;

//##################################################
//################# Door contact ###################
const int door_switch = 2;

void setup_switch() {
  pinMode(door_switch, INPUT_PULLUP);
}

bool door_is_open() {
  return (digitalRead(door_switch) == LOW);
}

//##################################################
//################# Keypad State Machine ###########
typedef enum {
  IDLE_DISARMED,
  WAIT_LEAVE,
  IDLE_ARMED,
  DOOR_OPENED,
  KEY_ENTERED
} states_type;

states_type Door_state;
char input[10] = "";
int index = 0;
bool server_key_is_valid = false;

//##################################################
//################# SETUP ##########################
void setup() {
  Serial.begin(9600);
  setup_keyboard();
  setup_status_leds();
  setup_switch();
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
          server_key_is_valid = false;
          
          Door_state = WAIT_LEAVE;
          Serial.print("state WAIT_LEAVE\n");
          digitalWrite(sled_rd, HIGH);
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

    // After alarm is armed, wait for person to leave.
    case WAIT_LEAVE:{
      // Wait for door to be opened and closed again.
      Serial.print("waiting for door to be opened.\n");
      while(!door_is_open())
        delay(50);
      Serial.print("waiting for door to be closed.\n");
      while(door_is_open())
        delay(50);

      Door_state = IDLE_ARMED;
      Serial.print("state IDLE_ARMED\n");
      digitalWrite(sled_rd, LOW);
    }
    break;

    // Armed state. Blink LED from time to time and
    // wait for door being opened.
    case IDLE_ARMED:{
      if (door_is_open())
      {

        // TODO notfy server about door open
        
        Door_state = DOOR_OPENED;
        Serial.print("state DOOR_OPENED\n");
        next_flash = millis() + INTRUDER_FLASH_PERIOD;
      }

      // blink status led
      if (millis() > next_flash)
      {

        // TODO periodically check if server is connected.

        if(true)
          flash_led(sled_rd, 1, 250);
        else
          flash_led(sled_rd, 3, 100);
        next_flash = millis() + ARMED_FLASH_PERIOD;
      }
    }
    break;

    // Door has been opened. Listen to keyboard inputs.
    case DOOR_OPENED:{
      
      digitalWrite(sled_ye, HIGH);
      char keycode = poll_keyboard();
      if(keycode != -1)
      {
        digitalWrite(sled_ye, LOW);
        input[index++] = keycode;
        if ((keycode == 'D') || (index == 9))
        {
          input[index] = '\0';
          index = 0;

          // TODO send key to server
          Serial.print(input);
          Serial.print(" has been sent to server for checking.\n");
          
          Door_state = KEY_ENTERED;
          Serial.print("state KEY_ENTERED\n");
        }
      }
      
      // blink status led
      if (millis() > next_flash)
      {
        flash_led(sled_rd, 1, 250);
        next_flash = millis() + INTRUDER_FLASH_PERIOD;
      }
    }
    break;

    // Key has been entered. Wait for response from server.
    case KEY_ENTERED:{

      // TODO make server check the key
      const char password[] = "1234D";
      if(strcmp(password, input) == 0)
        server_key_is_valid = true;

      if(server_key_is_valid)
      {
        Door_state = IDLE_DISARMED;
        Serial.print("state IDLE_DISARMED\n");
        flash_led(sled_ye, 1, 1000);
      } else {
        Door_state = DOOR_OPENED;
        Serial.print("state DOOR_OPENED\n");
        flash_led(sled_ye, 8, 50);
      }

      // blink status led
      if (millis() > next_flash)
      {
        flash_led(sled_rd, 1, 250);
        next_flash = millis() + INTRUDER_FLASH_PERIOD;
      }
    }
    break;
  }
}
