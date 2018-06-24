#include <WireLCD.h>
#include <Wire.h>
#include <RtcInterface.h>

//================== TESTBED FOR NEW CLASSES ================================


//============== BOILERPLATE FOR TESTING =============================


WireLCD lcd(0x26);
RtcInterface ds1307;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  lcd.init();

  ds1307.startStopControl(true);
}

TimeStruct setTimeBySerial()
{
  TimeStruct retval;
  retval.year = static_cast<uint32_t>(Serial.parseInt());
  retval.month = static_cast<uint8_t>(Serial.parseInt());
  retval.day = static_cast<uint8_t>(Serial.parseInt());
  retval.hour = static_cast<uint8_t>(Serial.parseInt());
  retval.minute = static_cast<uint8_t>(Serial.parseInt());
  retval.second = static_cast<uint8_t>(Serial.parseInt());

  if (retval.isValid())
    Serial.println(retval.toString());
  
  return retval;
}

void loop() {

  if (Serial.available()) {
    const TimeStruct new_time = setTimeBySerial();
    if (new_time.isValid())
      ds1307.setTime(new_time);
  }

  const TimeStruct rtc_time = ds1307.getTime();

  const String line0 = "Time: " + rtc_time.timeString();
  const String line1 = "Date: " + rtc_time.dateString();

  lcd.writeLine((uint8_t*)line0.c_str(), line0.length(), 0);
  lcd.writeLine((uint8_t*)line1.c_str(), line1.length(), 1);

  delay(200);
}
