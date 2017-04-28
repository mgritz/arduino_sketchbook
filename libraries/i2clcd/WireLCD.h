#ifndef WireLCD_h
#define WireLCD_h
#include "Arduino.h"

class WireLCD
{
public:
  WireLCD(uint8_t addr);
  void init();
  void writeLine(const uint8_t* buf, const uint8_t len, const uint8_t line);
private:
  uint8_t address;
  uint8_t lines;
  uint8_t cols;

  void lcd_toggle_enable(const uint8_t bits);
  void lcd_byte(const uint8_t bits, const uint8_t mode);
};

#endif
