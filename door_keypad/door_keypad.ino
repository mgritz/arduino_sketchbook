#include <Adafruit_PN532.h>

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
        return keycodes[row][col];
      }
    digitalWrite(rowPins[row], HIGH);
  }
  return -1;
}

//##################################################
//################# coms stuff #####################
//+++++++++++++++++ Protocol +++++++++++++++++++++++
const byte door_ms_alive = 'S';   // SYN, keepalive
const byte door_sm_ack = 'K';     // ACK, + door flag
// keys are represented by their actual ASCII char
const byte door_ms_key_res = 'R'; // key eval + flag
//+++++++++++++++++ Driver +++++++++++++++++++++++++
// We are running the standard serial interface 
// driver @ 
#define SERIAL_BOUDRATE 1200
// boud. The Server is required to transmit a
// keepalive every
#define SERIAL_KEEPALIVE_PERIODE 10000
// milliseconds.
unsigned long serial_keepalive_next_timeout = SERIAL_KEEPALIVE_PERIODE;

//##################################################
//################# Status LEDs ####################
const int sled_rd = 13;
const int sled_ye = 4;

const int beeper = 14;
bool beeper_active = false;

void setup_status_leds() {
  pinMode(sled_rd, OUTPUT);
  pinMode(sled_ye, OUTPUT);
  pinMode(beeper, OUTPUT);
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

const unsigned long LED_UPDATE_CYCLE = 1000;
void operate_status_led(){
  static unsigned long next_flash = LED_UPDATE_CYCLE;
  // time to flash LED
  if (next_flash < millis()){
    // keepalive timed out.
    if (serial_keepalive_next_timeout < millis())
      digitalWrite(sled_rd, HIGH);
    else{
      flash_led(sled_rd, 1, 100);
    }
    if (beeper_active){
      digitalWrite(beeper, HIGH);
      delay(250);
      digitalWrite(beeper, LOW);
    }
    next_flash = millis() + LED_UPDATE_CYCLE;
  }
}

//##################################################
//################# Door contact ###################
const int door_switch = 2;

void setup_switch() {
  pinMode(door_switch, INPUT_PULLUP);
}

bool door_is_open() {
  return (digitalRead(door_switch) == HIGH);
}

//##################################################
//################# NFC reader #####################
#define PN532_IRQ   (3)
#define PN532_RESET (17)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
bool nfc_connected = false;

void setup_nfc(){
  nfc.begin();
  nfc_connected = (nfc.getFirmwareVersion() != 0);
  // configure board to read RFID tags
  if (nfc_connected)
    nfc.SAMConfig();
}

// returns 0 if no card has been received.
// Otherwise, returns the card's 4-byte-UID converted
// little-endian to uint32_t
uint32_t nfc_waitCard(){
  static uint32_t last_nfc_key = 0;
  static unsigned long forget_timeout = 0;
  
  if (!nfc_connected) 
    return 0;

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;
  uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  
  uint8_t success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);

  if(success && (uidLength == 4)){
    // We probably have a Mifare Classic.
    uint32_t new_key = *((uint32_t*)uid);

    // check if this is new card and return value
    if((new_key != last_nfc_key) || (millis() >= forget_timeout)){
      last_nfc_key = new_key;
      forget_timeout = millis() + 5000;
      return new_key;
    }
  }

  return 0;
}

//##################################################
//################# SETUP ##########################
void setup() {
  Serial.begin(SERIAL_BOUDRATE);
  setup_keyboard();
  setup_status_leds();
  setup_switch();
  setup_nfc();
}
//##################################################
//################# RUN ############################
void loop() {
  static bool door_was_open = false;
  static bool next_is_key_byte = false;
  
  // show status LED if needed, take care of
  // server disconnecting
  operate_status_led();

  // handle keyboard and send code
  char keycode = poll_keyboard();
  if (keycode != -1){
    Serial.write(keycode);
    flash_led(sled_ye, 1, 50);
    beeper_active = false;
  }

  // handle NFC card being placed on reader
  uint32_t nfc_key = nfc_waitCard();
  if (nfc_key != 0){
    // write key as string and add 'D' for disarm
    Serial.print('C');
    Serial.print(nfc_key);
    Serial.print('D');
  }

  // check if door has been opened
  if ((!door_was_open) && (door_is_open())){
    char resp[2] = {door_sm_ack, 0x01};
    Serial.write(resp, 2);
  }
  door_was_open = door_is_open();

  // handle received bytes
  for (int r = 0; (r < 10) && (Serial.available() > 0); ++r) {  
    byte rx = Serial.read();
    if (rx == -1) break;

    if(rx == door_ms_alive){
      // keepalive from server, respond with door state
        char resp[2] = {door_sm_ack, 0x00};
        resp[1] = door_is_open();
        Serial.write(resp, 2);

        serial_keepalive_next_timeout = millis() 
              + SERIAL_KEEPALIVE_PERIODE;
    } else if (rx == door_ms_key_res){
      // door key result, display as LED code
      next_is_key_byte = true;
    } else if (next_is_key_byte){
      if (rx == 0){      
        flash_led(sled_ye, 8, 50);
      } else if (rx == 1) {
        flash_led(sled_ye, 2, 250);
        beeper_active = false;
      } else if (rx == 2) {
        beeper_active = true;
      }
      next_is_key_byte = false;
    }
  }
  delay(2);
}

