#include <WireLCD.h>
#include <Wire.h>

WireLCD lcd(0x26);

void setup(){
  Wire.begin();
  lcd.init();
}

String foobar("FooBar");

void loop(){
  lcd.writeLine((uint8_t*)foobar.c_str(), foobar.length(), 0);
  delay(1000);
}
