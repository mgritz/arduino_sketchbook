#include "Arduino.h"
#include "WireLCD.h"
#include <Wire.h>

#define LCD_CHR 1 // Mode - Sending data
#define LCD_CMD 0 // Mode - Sending command
#define LCD_BACKLIGHT 0x08  // On
#define ENABLE 0x04 // Enable bit
#define E_PULSE 500 // Timing constants (ms)
#define E_DELAY 500

// LCD RAM addresses for the four lines.
const uint8_t LINECODES[] = { 0x80, 0xC0, 0x94, 0xD4 };


WireLCD::WireLCD(uint8_t addr, uint8_t lines, uint8_t cols)
  : address(addr), lines(lines), cols(cols)
{
}

void WireLCD::init()
{
  lcd_byte(0x33, LCD_CMD);  // init
  lcd_byte(0x32, LCD_CMD);  // init
  lcd_byte(0x06, LCD_CMD);  // Cursor direction
  lcd_byte(0x0C, LCD_CMD);  // Display on, cursor off, Blink off.
  lcd_byte(0x28, LCD_CMD);  // Data length, number of lines, font
  lcd_byte(0x01, LCD_CMD);  // Clear
  delayMicroseconds(E_DELAY);
}

void WireLCD::lcd_toggle_enable(const uint8_t bits)
{
  delayMicroseconds(E_DELAY);
  Wire.beginTransmission(address);
  Wire.write(bits | ENABLE);
  Wire.endTransmission();
  delayMicroseconds(E_PULSE);
  Wire.beginTransmission(address);
  Wire.write(bits & (~ENABLE));
  Wire.endTransmission();
  delayMicroseconds(E_DELAY);
}

void WireLCD::lcd_byte(const uint8_t bits, const uint8_t mode)
{
  uint8_t bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
  uint8_t bits_low = mode | ((bits<<4) & 0xF0) | LCD_BACKLIGHT;
  Wire.beginTransmission(address);
  Wire.write(bits_high);
  Wire.endTransmission();
  lcd_toggle_enable(bits_high);
  Wire.beginTransmission(address);
  Wire.write(bits_low);
  Wire.endTransmission();
  lcd_toggle_enable(bits_low);
}

void WireLCD::writeLine(const uint8_t* buf, const uint8_t len, const uint8_t line)
{
  if(line >= lines || line < 0)
    return;

  char i = 0;
  lcd_byte(LINECODES[line], LCD_CMD);
  for(i = 0; i < cols; ++i)
  {
    if( i < len)
    {
      lcd_byte(buf[i], LCD_CHR);
    } else {
      lcd_byte(' ', LCD_CHR);
    }
  }
}
