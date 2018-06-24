#ifndef RTCINTERFACE_H
#define RTCINTERFACE_H

#include "Arduino.h"

class TimeStruct{
public:
  typedef enum {SUN = 1, MON, TUE, WED, THU, FRI, SAT} DayOfWeek;
  TimeStruct() 
    : year(0), month(0), day(0), dow(SUN), hour(0), minute(0), second(0) {}

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

  bool readRegisters();
  bool writeRegisters();
};

#endif // include guard