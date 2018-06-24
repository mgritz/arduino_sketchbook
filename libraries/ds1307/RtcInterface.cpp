#include "Arduino.h"
#include "RtcInterface.h"

#include <Wire.h>

bool RtcInterface::readRegisters() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(static_cast<uint8_t>(0));
  Wire.endTransmission();
  Wire.requestFrom(i2cAddress, static_cast<uint8_t>(8));
  for(int i = 0; Wire.available() && i < 8; ++i)
    buf[i] = Wire.read();
  return true;
}

bool RtcInterface::writeRegisters() {
  Wire.beginTransmission(i2cAddress);
  Wire.write(static_cast<uint8_t>(0));
  for (int i = 0; i < 8; ++i)
    Wire.write(buf[i]);
  Wire.endTransmission();
  return true;
}

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