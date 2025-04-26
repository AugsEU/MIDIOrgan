#include <ScreenDisplay.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// LiquidCrystal_I2C gLcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void LcdInit()
{
    // Serial.begin(9600); // breaks midi
    // gLcd.init();
    // gLcd.noBacklight();  
    // gLcd.setCursor(0, 0);
    // gLcd.print("Hello world");
}

void WriteToLcd(const char* message)
{
    // Serial.begin(9600); // breaks midi
    // gLcd.print(message);
}