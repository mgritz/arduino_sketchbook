#include <WireLCD.h>
#include <Wire.h>

//================== TESTBED FOR NEW CLASSES ================================
class TimeStruct{
public:
  typedef enum {SUN = 1, MON, TUE, WED, THU, FRI, SAT} DayOfWeek;
  TimeStruct() : year(0), month(0), day(0), dow(SUN), hour(0), minute(0), second(0) {}
  uint32_t year;
  uint8_t month;
  uint8_t day;
  DayOfWeek dow;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  String timeString() const {
    return String(hour) + ":" + String(minute) + ":" + String(second);
  }

  String dateString() const {
    return String(year) + "-" + String(month) + "-" + String(day);
  }

  String toString() const { return dateString() + " " + timeString(); }

  bool isValid() const { 
    return (year > 0) && (month >= 1) && (month <= 12)
      && (day >= 1) && (day <= 31);
  }
};

class RtcInterface{
public:
  void startStopControl(bool run);
  bool setTime(const TimeStruct& n);
  TimeStruct getTime();

private:
  const uint8_t i2cAddress = 0b1101000;
  uint8_t buf[8];

  bool readRegisters() {
    Wire.beginTransmission(i2cAddress);
    Wire.write(static_cast<uint8_t>(0));
    Wire.endTransmission();
    Wire.requestFrom(i2cAddress, static_cast<uint8_t>(8));
    for(int i = 0; Wire.available() && i < 8; ++i)
      buf[i] = Wire.read();
    return true;
  }

  bool writeRegisters() {
    Wire.beginTransmission(i2cAddress);
    Wire.write(static_cast<uint8_t>(0));
    for (int i = 0; i < 8; ++i)
      Wire.write(buf[i]);
    Wire.endTransmission();
    return true;
  }
};

void RtcInterface::startStopControl(bool run)
{
    Wire.beginTransmission(i2cAddress);
    Wire.write(static_cast<uint8_t>(0));
    Wire.endTransmission();
    Wire.requestFrom(i2cAddress, static_cast<uint8_t>(1));
    uint8_t b = Wire.read();

    if (run)
      b &= ~(0x80);
    else
      b |= 0x80;

    Wire.beginTransmission(i2cAddress);
    Wire.write(static_cast<uint8_t>(0));
    Wire.write(b);
    Wire.endTransmission();
}

bool RtcInterface::setTime(const TimeStruct& n)
{
  if(!n.isValid())
    return false;

  buf[0] = (n.second % 10) | ((((n.second / 10) % 10) << 4) & 0x70);
  buf[1] = (n.minute % 10) | ((((n.minute / 10) % 10) << 4) & 0x70);
  buf[2] = (n.hour% 10) | ((((n.hour/ 10) % 10) << 4) & 0x30); // always 24h
  buf[3] = static_cast<uint8_t>(n.dow);
  buf[4] = (n.day % 10) | ((((n.day / 10) % 10) << 4) & 0x30);
  buf[5] = (n.month % 10) | ((((n.month / 10) % 10) << 4) & 0x10);
  buf[6] = (n.year % 10) | ((((n.year / 10) % 10) << 4) & 0xF0);
  buf[7] = 0;

  return writeRegisters();
}

TimeStruct RtcInterface::getTime()
{
  TimeStruct r;
  if(!readRegisters())
    return r;

  r.second = (buf[0] & 0xF) + ((buf[0] >> 4) & 0x7) * 10;
  r.minute = (buf[1] & 0xF) + ((buf[1] >> 4) & 0x7) * 10;
  if (buf[2] & 0x40) { // 12h mode
    r.hour = (buf[2] & 0xF) + ((buf[2] >> 4) & 0x1) * 10;
    if (buf[2] & 0x20) r.hour += 12;
  } else // 24h mode
    r.hour = (buf[2] & 0xF) + ((buf[2] >> 4) & 0x3) * 10;
  r.dow = static_cast<TimeStruct::DayOfWeek>(buf[3] & 0x07);
  r.day = (buf[4] & 0xF) + ((buf[4] >> 4) & 0x3) * 10;
  r.month = (buf[5] & 0xF) + ((buf[5] >> 4) & 0x1) * 10;
  r.year = (buf[6] & 0xF) + ((buf[6] >> 4) & 0xF) * 10;

  return r;
}


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
