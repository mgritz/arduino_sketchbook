#include <WireLCD.h>
#include <Wire.h>

WireLCD lcd(0x27, 4, 20);

void setup(){
  Wire.begin();
  lcd.init();
}

String foobar("ABCDEFGHIJKLMNOPQRTU");

void loop(){
  lcd.writeLine((uint8_t*)foobar.c_str(), foobar.length(), 3);
  for(int i = 0; i < 100; ++i) {
    lcd.percentBar(0, i);
    lcd.percentBar(1, 100-i, '#', true);
    delay(500);
  }
}
